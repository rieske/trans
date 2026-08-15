#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/StackMachine.h"
#include "codegen/ATandTInstructionSet.h"
#include "codegen/IntelInstructionSet.h"
#include "codegen/IrStringTable.h"

#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

using testing::StrEq;
using namespace codegen;

class StackMachineTest: public testing::Test {
public:
    StackMachineTest() :
            registers { std::make_unique<Amd64Registers>() },
            generalPurposeRegisters { registers->getGeneralPurposeRegisters() },
            rax { generalPurposeRegisters[0] },
            rbx { generalPurposeRegisters[1] },
            rcx { generalPurposeRegisters[2] }
    {
    }

protected:
    int n(std::string_view text) {
        return names.intern(text);
    }

    Value intValue(std::string name) {
        return { names.intern(std::move(name)), 0, Type::INTEGRAL, 8 };
    }

    void expectCode(std::string expectedCode) {
        EXPECT_THAT(assemblyCode.str(), StrEq(expectedCode));
    }

    void expectRegisterContains(Register* reg, Value val) {
        EXPECT_TRUE(reg->getValue() != nullptr);
        EXPECT_EQ(reg->getValue()->id(), val.id());
    }

    Procedure testProc(std::string name, std::vector<Value> locals = {}, std::vector<Value> args = {},
            bool memoryReturn = false, bool variadic = false) {
        Procedure procedure;
        procedure.name = names.intern(std::move(name));
        procedure.frame.locals = std::move(locals);
        procedure.frame.arguments = std::move(args);
        procedure.memoryReturn = memoryReturn;
        procedure.variadic = variadic;
        internProcedureTemps(names, procedure);
        return procedure;
    }

    IrStringTable names;
    ATandTInstructionSet att;
    IntelInstructionSet intel;
    Amd64Registers extraRegs;
    std::unique_ptr<Amd64Registers> registers;
    std::stringstream assemblyCode { };

    std::vector<Register*> generalPurposeRegisters;

    Register* rax;
    Register* rbx;
    Register* rcx;

    Value v1 { names.intern("v1"), 0, Type::INTEGRAL, 8 };
    Value v2 { names.intern("v2"), 1, Type::INTEGRAL, 8 };
    Value v3 { names.intern("v3"), 2, Type::INTEGRAL, 8 };

private:

};

TEST_F(StackMachineTest, procedureCall_doesNotPushUnusedCallerSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, att, *registers, names };

    stackMachine.callProcedure(n("procedure"));

    expectCode("\txorq %rax, %rax\n"
            "\tcall procedure@plt\n");
}

TEST_F(StackMachineTest, functionAddress_leaDefinedProcedure) {
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    Value fp = intValue("fp");
    stackMachine.registerDefinedProcedure(n("foo"));
    stackMachine.startProcedure(testProc("foo", { fp }, { }));
    assemblyCode.str("");

    stackMachine.functionAddress(n("foo"), n("fp"));

    expectCode("\tlea rax, [rel $foo]\n");
}

TEST_F(StackMachineTest, functionAddress_loadsExternViaGot) {
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    Value fp = intValue("fp");
    stackMachine.registerDefinedProcedure(n("proc"));
    stackMachine.startProcedure(testProc("proc", { fp }, { }));
    assemblyCode.str("");

    stackMachine.functionAddress(n("printf"), n("fp"));

    expectCode("\tmov rax, [rel $printf wrt ..got]\n");
}

TEST_F(StackMachineTest, assignLabelAddress_leaPoolLabel) {
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    Value s = intValue("s");
    stackMachine.startProcedure(testProc("proc", { s }, { }));
    assemblyCode.str("");

    stackMachine.assignLabelAddress(n("L$str1"), n("s"));

    expectCode("\tlea rax, [rel $L$str1]\n");
}

TEST_F(StackMachineTest, callProcedure_sameTuDoesNotUsePlt) {
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.registerDefinedProcedure(n("foo"));

    stackMachine.callProcedure(n("foo"));

    expectCode("\txor rax, rax\n"
            "\tcall $foo\n");
}

TEST_F(StackMachineTest, callProcedure_externUsesPlt) {
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };

    stackMachine.callProcedure(n("printf"));

    expectCode("\txor rax, rax\n"
            "\tcall $printf wrt ..plt\n");
}

// Target already in a callee-saved reg survives spillCallerSavedRegisters → mov to r10.
TEST_F(StackMachineTest, callProcedureIndirect_movesRegisterTargetToR10) {
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    Value junk = intValue("junk");
    Value fp = { names.intern("fp"), 1, Type::INTEGRAL, 8 };
    stackMachine.startProcedure(testProc("proc", { junk, fp }, { }));
    // Occupy rax so the next functionAddress binds fp to rbx (callee-saved).
    stackMachine.functionAddress(n("a"), n("junk"));
    stackMachine.functionAddress(n("foo"), n("fp"));
    assemblyCode.str("");

    stackMachine.callProcedureIndirect(n("fp"));

    // Spill junk from rax (retrieval reg), then mov callee-saved fp → r10 and call.
    expectCode("\tmov [rbp + -16], rax\n"
            "\txor rax, rax\n"
            "\tmov r10, rbx\n"
            "\tcall r10\n");
}

TEST_F(StackMachineTest, callProcedureIndirect_loadsMemoryTargetToR10) {
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    Value fp = intValue("fp");
    stackMachine.startProcedure(testProc("proc", { fp }, { }));
    assemblyCode.str("");

    stackMachine.callProcedureIndirect(n("fp"));

    // Local starts in a frame slot; load into r10 then indirect call.
    EXPECT_THAT(assemblyCode.str(), testing::HasSubstr("call r10"));
    EXPECT_THAT(assemblyCode.str(), testing::HasSubstr("r10"));
}

TEST_F(StackMachineTest, procedureCall_storesAllDirtyCallerSavedRegisters) {
    Value value { names.intern("value"), 0, Type::INTEGRAL, 4 };
    for (auto &reg : registers->getGeneralPurposeRegisters()) {
        reg->assign(&value);
    }
    StackMachine stackMachine { &assemblyCode, att, *registers, names };

    stackMachine.callProcedure(n("procedure"));

    expectCode("\tmovl %eax, (%rsp)\n"
            "\tmovl %ecx, (%rsp)\n"
            "\tmovl %edx, (%rsp)\n"
            "\tmovl %esi, (%rsp)\n"
            "\tmovl %edi, (%rsp)\n"
            "\tmovl %r8d, (%rsp)\n"
            "\tmovl %r9d, (%rsp)\n"
            "\tmovl %r10d, (%rsp)\n"
            "\tmovl %r11d, (%rsp)\n"
            "\txorq %rax, %rax\n"
            "\tcall procedure@plt\n");
}

// Variadic ABI: AL must be 0 when no vector args are passed (e.g. printf with integers only)
TEST_F(StackMachineTest, procedureCall_clearsRaxForVariadicAlRequirement) {
    StackMachine stackMachine { &assemblyCode, att, extraRegs, names };
    Value value = intValue("value");
    stackMachine.startProcedure(testProc("proc", { value }, { }));
    assemblyCode.str("");

    stackMachine.procedureArgument(value.id());
    stackMachine.callProcedure(n("printf"));

    expectCode("\tmovq -16(%rbp), %rdi\n"
            "\txorq %rax, %rax\n"
            "\tcall printf@plt\n");
}

TEST_F(StackMachineTest, procedureStart_storesCalleeSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, att, *registers, names };

    stackMachine.startProcedure(testProc("proc", { }, { }));

    expectCode(".globl proc\n"
            "proc:\n"
            "\tpushq %rbp\n"
            "\tmovq %rsp, %rbp\n"
            "\tsubq $8, %rsp\n"
            "\tpushq %rbx\n"
            "\tpushq %r12\n"
            "\tpushq %r13\n"
            "\tpushq %r14\n"
            "\tpushq %r15\n");
}

TEST_F(StackMachineTest, procedureReturn_returnsWithNoCalleeRegistersSaved) {
    StackMachine stackMachine { &assemblyCode, att, *registers, names };

    stackMachine.returnFromProcedure();

    expectCode("\tleave\n"
            "\tret\n");
}

TEST_F(StackMachineTest, procedureReturn_popsCalleeSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, att, *registers, names };
    stackMachine.startProcedure(testProc("proc", { }, { }));
    assemblyCode.str("");

    stackMachine.returnFromProcedure();

    expectCode("\tleaq -48(%rbp), %rsp\n"
            "\tpopq %r15\n"
            "\tpopq %r14\n"
            "\tpopq %r13\n"
            "\tpopq %r12\n"
            "\tpopq %rbx\n"
            "\tleave\n"
            "\tret\n");
}

TEST_F(StackMachineTest, procedureArgumentPassing_firstIntegerArgumentIsPassedInRDI) {
    StackMachine stackMachine { &assemblyCode, att, extraRegs, names };
    Value value = intValue("value");
    stackMachine.startProcedure(testProc("proc", { value }, { }));
    assemblyCode.str("");

    stackMachine.procedureArgument(value.id());
    stackMachine.callProcedure(n("procedure"));

    expectCode("\tmovq -16(%rbp), %rdi\n"
            "\txorq %rax, %rax\n"
            "\tcall procedure@plt\n");
}

// Odd number of stack args needs 8-byte padding so RSP is 16-byte aligned before call
TEST_F(StackMachineTest, procedureCall_padsStackForOddNumberOfStackArguments) {
    StackMachine stackMachine { &assemblyCode, att, extraRegs, names };
    std::vector<Value> locals;
    for (int i = 0; i < 7; ++i) {
        locals.push_back({ names.intern("a" + std::to_string(i)), i, Type::INTEGRAL, 8 });
    }
    stackMachine.startProcedure(testProc("proc", locals, { }));
    assemblyCode.str("");

    for (const auto& local : locals) {
        stackMachine.procedureArgument(local.id());
    }
    stackMachine.callProcedure(n("procedure"));

    expectCode("\tmovq -64(%rbp), %rdi\n"
            "\tmovq -56(%rbp), %rsi\n"
            "\tmovq -48(%rbp), %rdx\n"
            "\tmovq -40(%rbp), %rcx\n"
            "\tmovq -32(%rbp), %r8\n"
            "\tmovq -24(%rbp), %r9\n"
            "\tsubq $16, %rsp\n"
            "\tmovq -16(%rbp), %rax\n"
            "\tmovq %rax, (%rsp)\n"
            "\txorq %rax, %rax\n"
            "\tcall procedure@plt\n"
            "\taddq $16, %rsp\n");
}

TEST_F(StackMachineTest, setScopeRejectsDuplicateInternId) {
    StackMachine stackMachine { &assemblyCode, att, *registers, names };
    stackMachine.setScope({ v1 });
    EXPECT_THROW(stackMachine.setScope({ v1 }), std::logic_error);
}

TEST_F(StackMachineTest, sub_reg_reg) {
    // given
    rax->assign(&v1);
    rbx->assign(&v2);

    StackMachine stackMachine { &assemblyCode, att, *registers, names };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.id(), v2.id(), v3.id());

    // then
    expectCode("\tmovq %rax, %rcx\n"
            "\tsubq %rbx, %rcx\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v2);
    expectRegisterContains(rcx, v3);
}

TEST_F(StackMachineTest, sub_reg_mem) {
    // given
    rax->assign(&v1);

    StackMachine stackMachine { &assemblyCode, att, *registers, names };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.id(), v2.id(), v3.id());

    // then
    expectCode("\tmovq %rax, %rbx\n"
            "\tmovq 8(%rsp), %rcx\n"
            "\tsubq %rcx, %rbx\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, sub_mem_reg) {
    // given
    rax->assign(&v2);

    StackMachine stackMachine { &assemblyCode, att, *registers, names };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.id(), v2.id(), v3.id());

    // then
    expectCode("\tmovq (%rsp), %rbx\n"
            "\tsubq %rax, %rbx\n");

    expectRegisterContains(rax, v2);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, sub_mem_mem) {
    // given
    StackMachine stackMachine { &assemblyCode, att, *registers, names };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.id(), v2.id(), v3.id());

    // then
    expectCode("\tmovq (%rsp), %rax\n"
            "\tmovq 8(%rsp), %rbx\n"
            "\tsubq %rbx, %rax\n");

    expectRegisterContains(rax, v3);
}

TEST_F(StackMachineTest, add_reg_reg) {
    // given
    rax->assign(&v1);
    rbx->assign(&v2);

    StackMachine stackMachine { &assemblyCode, att, *registers, names };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.id(), v2.id(), v3.id());

    // then
    expectCode("\tmovq %rax, %rcx\n"
            "\taddq %rbx, %rcx\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v2);
    expectRegisterContains(rcx, v3);
}

TEST_F(StackMachineTest, add_reg_mem) {
    // given
    rax->assign(&v1);

    StackMachine stackMachine { &assemblyCode, att, *registers, names };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.id(), v2.id(), v3.id());

    // then
    expectCode("\tmovq %rax, %rbx\n"
            "\tmovq 8(%rsp), %rcx\n"
            "\taddq %rcx, %rbx\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, add_mem_reg) {
    // given
    rax->assign(&v2);

    StackMachine stackMachine { &assemblyCode, att, *registers, names };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.id(), v2.id(), v3.id());

    // then
    expectCode("\tmovq (%rsp), %rbx\n"
            "\taddq %rax, %rbx\n");

    expectRegisterContains(rax, v2);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, add_mem_mem) {
    // given
    StackMachine stackMachine { &assemblyCode, att, *registers, names };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.id(), v2.id(), v3.id());

    // then
    expectCode("\tmovq (%rsp), %rax\n"
            "\tmovq 8(%rsp), %rbx\n"
            "\taddq %rbx, %rax\n");

    expectRegisterContains(rax, v3);
}

TEST_F(StackMachineTest, variadicPrologueDumpsGpAndXmmSaveArea) {
    Value named { names.intern("n"), 0, Type::INTEGRAL, 4 };
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("varfn", {}, { named }, false, true));

    const std::string code = assemblyCode.str();
    EXPECT_THAT(code, testing::HasSubstr("rdi"));
    EXPECT_THAT(code, testing::HasSubstr("xmm0"));
}

TEST_F(StackMachineTest, nonVariadicPrologueDoesNotDumpXmmSaveArea) {
    Value named { names.intern("n"), 0, Type::INTEGRAL, 4 };
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("plain", {}, { named }, false, false));

    const std::string code = assemblyCode.str();
    EXPECT_THAT(code, testing::Not(testing::HasSubstr("xmm0")));
}

// Floating args go in xmm0.. and set AL for variadic callees (SysV).
TEST_F(StackMachineTest, intelFloatingArgumentUsesXmmAndSetsAl) {
    Value d { names.intern("d"), 0, Type::FLOATING, 8 };
    Value fmt { names.intern("fmt"), 1, Type::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("caller", { d, fmt }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(fmt.id());
    stackMachine.procedureArgument(d.id());
    stackMachine.callProcedure(n("printf"));

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, testing::HasSubstr("xmm0"));
    EXPECT_THAT(code, testing::HasSubstr("call $printf"));
    // AL = number of vector registers used (mov to rax/eax both set AL).
    EXPECT_TRUE(code.find("mov rax, 1") != std::string::npos
            || code.find("mov eax, 1") != std::string::npos);
}

// float32 call arg is movd into xmm, not movq.
TEST_F(StackMachineTest, intelFloat32CallArgUsesMovd) {
    Value fmt { names.intern("fmt"), 0, Type::INTEGRAL, 8 };
    Value f { names.intern("f"), 1, Type::FLOATING, 4 };
    Value code { names.intern("code"), 2, Type::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("caller", { fmt, f, code }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(fmt.id());
    stackMachine.procedureArgument(f.id());
    stackMachine.procedureArgument(code.id());
    stackMachine.callProcedure(n("callee"));

    std::string codeAsm = assemblyCode.str();
    EXPECT_THAT(codeAsm, testing::HasSubstr("movd xmm0"));
    EXPECT_THAT(codeAsm, testing::Not(testing::HasSubstr("movq xmm0")));
}

// AL counts xmm0..xmm7 only; a ninth float does not set AL to 9.
TEST_F(StackMachineTest, intelNinthFloatDoesNotCountTowardAl) {
    std::vector<Value> locals;
    for (int i = 0; i < 9; ++i) {
        locals.push_back({ names.intern("f" + std::to_string(i)), i, Type::FLOATING, 8 });
    }
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("caller", locals, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    for (int i = 0; i < 9; ++i) {
        stackMachine.procedureArgument(n("f" + std::to_string(i)));
    }
    stackMachine.callProcedure(n("callee"));

    std::string code = assemblyCode.str();
    EXPECT_TRUE(code.find("mov rax, 8") != std::string::npos
            || code.find("mov eax, 8") != std::string::npos);
    EXPECT_TRUE(code.find("mov rax, 9") == std::string::npos
            && code.find("mov eax, 9") == std::string::npos);
}

TEST_F(StackMachineTest, intelMultiWordArgumentCopiesOntoStack) {
    Value big { names.intern("big"), 0, Type::INTEGRAL, 24 };
    std::vector<Value> locals { big };
    std::vector<Value> namedArgs;
    for (int i = 0; i < 6; ++i) {
        namedArgs.push_back({ names.intern("a" + std::to_string(i)), i + 1, Type::INTEGRAL, 8 });
        locals.push_back(namedArgs.back());
    }
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("caller", locals, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    for (const auto& a : namedArgs) {
        stackMachine.procedureArgument(a.id());
    }
    stackMachine.procedureArgument(big.id());
    stackMachine.callProcedure(n("callee"));

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, testing::HasSubstr("sub rsp, 32"));
    EXPECT_THAT(code, testing::HasSubstr("add rsp, 32"));
}

TEST_F(StackMachineTest, intelCallWithMemoryReturnDestLeasIntoRdi) {
    Value dest { names.intern("dest"), 0, Type::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("caller", { dest }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.callProcedure(n("make"), dest.id());

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, testing::HasSubstr("lea rdi"));
    EXPECT_THAT(code, testing::HasSubstr("call $make"));
}

TEST_F(StackMachineTest, intelMemoryReturnCopiesObjectToSretAndLeavesPointerInRax) {
    Value ret { names.intern("ret"), 0, Type::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("make", { ret }, {}, true));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.returnFromProcedure(ret.id());

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, testing::HasSubstr("rax"));
    EXPECT_THAT(code, testing::Not(testing::HasSubstr("mov rdx, rax")));
}

TEST_F(StackMachineTest, intelFloat32LvalueAssignUsesDword) {
    Value f { names.intern("f"), 0, Type::FLOATING, 4 };
    Value p { names.intern("p"), 1, Type::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("storef", { f, p }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.lvalueAssign(n("f"), n("p"));

    EXPECT_THAT(assemblyCode.str(), testing::HasSubstr("dword"));
}

// SSE path for double add: load bits, addsd, park result.
TEST_F(StackMachineTest, intelFloatingAddUsesAddsd) {
    Value a { names.intern("a"), 0, Type::FLOATING, 8 };
    Value b { names.intern("b"), 1, Type::FLOATING, 8 };
    Value r { names.intern("r"), 2, Type::FLOATING, 8 };
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("fadd", { a, b, r }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.add(n("a"), n("b"), n("r"));

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, testing::HasSubstr("addsd"));
    EXPECT_THAT(code, testing::HasSubstr("xmm0"));
    EXPECT_THAT(code, testing::HasSubstr("xmm1"));
}

TEST_F(StackMachineTest, intelFloat32AddUsesAddss) {
    Value a { names.intern("a"), 0, Type::FLOATING, 4 };
    Value b { names.intern("b"), 1, Type::FLOATING, 4 };
    Value r { names.intern("r"), 2, Type::FLOATING, 4 };
    StackMachine stackMachine { &assemblyCode, intel, extraRegs, names };
    stackMachine.startProcedure(testProc("faddss", { a, b, r }, {}));
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.add(n("a"), n("b"), n("r"));

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, testing::HasSubstr("addss"));
    EXPECT_THAT(code, testing::Not(testing::HasSubstr("addsd")));
}

}
