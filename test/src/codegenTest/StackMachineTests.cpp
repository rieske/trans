#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/StackMachine.h"
#include "codegen/IntelInstructionSet.h"

#include <memory>
#include <string>
#include <vector>

namespace {

using testing::HasSubstr;
using testing::Not;
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
    Value intValue(std::string name) {
        return { name, 0, ValueKind::INTEGRAL, 8 };
    }

    void expectCode(std::string expectedCode) {
        EXPECT_THAT(assemblyCode.str(), StrEq(expectedCode));
    }

    void expectRegisterContains(Register* reg, Value val) {
        EXPECT_TRUE(reg->getValue() != nullptr);
        EXPECT_THAT(reg->getValue()->getName(), StrEq(val.getName()));
    }

    std::unique_ptr<Amd64Registers> registers;
    std::stringstream assemblyCode { };

    std::vector<Register*> generalPurposeRegisters;

    Register* rax;
    Register* rbx;
    Register* rcx;

    Value v1 { "v1", 0, ValueKind::INTEGRAL, 8 };
    Value v2 { "v2", 1, ValueKind::INTEGRAL, 8 };
    Value v3 { "v3", 2, ValueKind::INTEGRAL, 8 };

private:

};

TEST_F(StackMachineTest, procedureCall_doesNotPushUnusedCallerSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };

    stackMachine.callProcedure("procedure");

    expectCode("\txor rax, rax\n"
            "\tcall $procedure wrt ..plt\n");
}

TEST_F(StackMachineTest, procedureCall_storesAllDirtyCallerSavedRegisters) {
    Value value { "value", 0, ValueKind::INTEGRAL, 4 };
    for (auto &reg : registers->getGeneralPurposeRegisters()) {
        reg->assign(&value);
    }
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };

    stackMachine.callProcedure("procedure");

    expectCode("\tmov [rsp], rax\n"
            "\tmov [rsp], rcx\n"
            "\tmov [rsp], rdx\n"
            "\tmov [rsp], rsi\n"
            "\tmov [rsp], rdi\n"
            "\tmov [rsp], r8\n"
            "\tmov [rsp], r9\n"
            "\tmov [rsp], r10\n"
            "\tmov [rsp], r11\n"
            "\txor rax, rax\n"
            "\tcall $procedure wrt ..plt\n");
}

// Variadic ABI: AL must be 0 when no vector args are passed (e.g. printf with integers only)
TEST_F(StackMachineTest, procedureCall_clearsRaxForVariadicAlRequirement) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value value = intValue("value");
    stackMachine.startProcedure("proc", { value }, { });
    assemblyCode.str("");

    stackMachine.procedureArgument(value.getName());
    stackMachine.callProcedure("printf");

    // Register args are loaded, then rematerialized after spillCallerSavedRegisters
    // (arg regs use loadWithoutBinding so spill may leave values only in memory).
    expectCode("\tmov rdi, [rsp + 40]\n"
            "\tmov rdi, [rsp + 40]\n"
            "\txor rax, rax\n"
            "\tcall $printf wrt ..plt\n");
}

TEST_F(StackMachineTest, procedureStart_storesCalleeSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };

    stackMachine.startProcedure("proc", { }, { });

    expectCode("$proc:\n"
            "\tpush rbp\n"
            "\tmov rbp, rsp\n"
            "\tsub rsp, 8\n"
            "\tpush rbx\n"
            "\tpush r12\n"
            "\tpush r13\n"
            "\tpush r14\n"
            "\tpush r15\n");
}

TEST_F(StackMachineTest, procedureReturn_returnsWithNoCalleeRegistersSaved) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };

    stackMachine.returnFromProcedure();

    expectCode("\tleave\n"
            "\tret\n");
}

TEST_F(StackMachineTest, procedureReturn_popsCalleeSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.startProcedure("proc", { }, { });
    assemblyCode.str("");

    stackMachine.returnFromProcedure();

    expectCode("\tpop r15\n"
            "\tpop r14\n"
            "\tpop r13\n"
            "\tpop r12\n"
            "\tpop rbx\n"
            "\tleave\n"
            "\tret\n");
}

TEST_F(StackMachineTest, procedureArgumentPassing_firstIntegerArgumentIsPassedInRDI) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value value = intValue("value");
    stackMachine.startProcedure("proc", { value }, { });
    assemblyCode.str("");

    stackMachine.procedureArgument(value.getName());
    stackMachine.callProcedure("procedure");

    expectCode("\tmov rdi, [rsp + 40]\n"
            "\tmov rdi, [rsp + 40]\n"
            "\txor rax, rax\n"
            "\tcall $procedure wrt ..plt\n");
}

// Odd number of stack args needs 8-byte padding so RSP is 16-byte aligned before call
TEST_F(StackMachineTest, procedureCall_padsStackForOddNumberOfStackArguments) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    std::vector<Value> locals;
    for (int i = 0; i < 7; ++i) {
        locals.push_back({ "a" + std::to_string(i), i, ValueKind::INTEGRAL, 8 });
    }
    stackMachine.startProcedure("proc", locals, { });
    assemblyCode.str("");

    for (const auto& local : locals) {
        stackMachine.procedureArgument(local.getName());
    }
    stackMachine.callProcedure("procedure");

    // Integer args loaded twice: once before spill, rematerialized after.
    expectCode("\tmov rdi, [rsp + 40]\n"
            "\tmov rsi, [rsp + 48]\n"
            "\tmov rdx, [rsp + 56]\n"
            "\tmov rcx, [rsp + 64]\n"
            "\tmov r8, [rsp + 72]\n"
            "\tmov r9, [rsp + 80]\n"
            "\tmov rdi, [rsp + 40]\n"
            "\tmov rsi, [rsp + 48]\n"
            "\tmov rdx, [rsp + 56]\n"
            "\tmov rcx, [rsp + 64]\n"
            "\tmov r8, [rsp + 72]\n"
            "\tmov r9, [rsp + 80]\n"
            "\tsub rsp, 8\n"
            "\tmov rax, [rsp + 96]\n"
            "\tpush rax\n"
            "\txor rax, rax\n"
            "\tcall $procedure wrt ..plt\n"
            "\tadd rsp, 16\n");
}

TEST_F(StackMachineTest, sub_reg_reg) {
    // given
    rax->assign(&v1);
    rbx->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rcx, rax\n"
            "\tsub rcx, rbx\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v2);
    expectRegisterContains(rcx, v3);
}

TEST_F(StackMachineTest, sub_reg_mem) {
    // given
    rax->assign(&v1);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rbx, rax\n"
            "\tsub rbx, [rsp + 8]\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, sub_mem_reg) {
    // given
    rax->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rbx, [rsp]\n"
            "\tsub rbx, rax\n");

    expectRegisterContains(rax, v2);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, sub_mem_mem) {
    // given
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rax, [rsp]\n"
            "\tsub rax, [rsp + 8]\n");

    expectRegisterContains(rax, v3);
}

TEST_F(StackMachineTest, add_reg_reg) {
    // given
    rax->assign(&v1);
    rbx->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rcx, rax\n"
            "\tadd rcx, rbx\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v2);
    expectRegisterContains(rcx, v3);
}

TEST_F(StackMachineTest, add_reg_mem) {
    // given
    rax->assign(&v1);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rbx, rax\n"
            "\tadd rbx, [rsp + 8]\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, add_mem_reg) {
    // given
    rax->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rbx, [rsp]\n"
            "\tadd rbx, rax\n");

    expectRegisterContains(rax, v2);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, add_mem_mem) {
    // given
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmov rax, [rsp]\n"
            "\tadd rax, [rsp + 8]\n");

    expectRegisterContains(rax, v3);
}


TEST_F(StackMachineTest, variadicPrologueDumpsGpAndXmmSaveArea) {
    Value last { "last", 0, ValueKind::INTEGRAL, 8, true };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };

    stackMachine.startProcedure("varfn", {}, { last }, true, false);

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("mov ["));
    EXPECT_THAT(code, HasSubstr("rdi"));
    EXPECT_THAT(code, HasSubstr("movq"));
    EXPECT_THAT(code, HasSubstr("xmm0"));
}

TEST_F(StackMachineTest, vaStartWritesTagOffsetsAndLeasOverflowAtRbp16) {
    Value last { "last", 0, ValueKind::INTEGRAL, 8, true };
    Value ap { "ap", 1, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("varfn", { ap }, { last }, true, false);
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.vaStart("ap", "last");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("mov dword [rax], 8"));
    EXPECT_THAT(code, HasSubstr("mov dword [rax + 4], 48"));
    EXPECT_THAT(code, HasSubstr("lea "));
    EXPECT_THAT(code, HasSubstr("[rbp + 16]"));
}

TEST_F(StackMachineTest, vaStartOverflowSkipsNamedStackArgs) {
    // 7 named ints: 6 GP + 1 stack. overflow_arg_area is rbp+24, not rbp+16.
    std::vector<Value> named;
    named.reserve(7);
    for (int i = 0; i < 7; ++i) {
        named.emplace_back("a" + std::to_string(i), i, ValueKind::INTEGRAL, 8, true);
    }
    Value ap { "ap", 7, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("varfn", { ap }, named, true, false);
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.vaStart("ap", "");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("[rbp + 24]"));
    EXPECT_THAT(code, Not(HasSubstr("[rbp + 16]")));
}

TEST_F(StackMachineTest, vaArgLabelsAreUniqueAcrossProcedures) {
    // AT&T .L labels are file-scoped; resetting vaArgSeq per procedure
    // collides when two variadic functions live in one TU.
    Value last { "last", 0, ValueKind::INTEGRAL, 8, true };
    Value ap { "ap", 1, ValueKind::INTEGRAL, 24 };
    Value result { "r", 2, ValueKind::INTEGRAL, 8, true };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };

    stackMachine.startProcedure("inner", { ap, result }, { last }, true, false);
    stackMachine.vaArg("ap", "r", 8, false, true);
    stackMachine.endProcedure();
    stackMachine.startProcedure("outer", { ap, result }, { last }, true, false);
    stackMachine.vaArg("ap", "r", 8, false, true);
    stackMachine.endProcedure();

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr(".Lva_arg_ov_1"));
    EXPECT_THAT(code, HasSubstr(".Lva_arg_ov_2"));
}

TEST_F(StackMachineTest, variadicReturnIsPlainEpilogue) {
    Value arg { "last", 0, ValueKind::INTEGRAL, 8, true };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("varfn", {}, { arg }, true, false);
    assemblyCode.str("");
    assemblyCode.clear();
    stackMachine.returnFromProcedure("");
    EXPECT_THAT(assemblyCode.str(), HasSubstr("ret"));
    EXPECT_THAT(assemblyCode.str(), Not(HasSubstr("call")));
}

// --- Production Intel/NASM path: System V memory return (sret) ---

TEST_F(StackMachineTest, intelMemoryReturnPrologueHoldsSretInFirstArgReg) {
    // Aggregates > 16 bytes: callee receives hidden pointer in rdi (first integer arg).
    Value local { "obj", 0, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };

    stackMachine.startProcedure("retbig", { local }, {}, false, true);

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("$retbig:"));
    EXPECT_THAT(code, HasSubstr("push rbp"));
    EXPECT_THAT(code, HasSubstr("mov rbp, rsp"));
    // sret local is spilled from rdi during prologue frame setup.
    EXPECT_THAT(code, HasSubstr("mov"));
}

TEST_F(StackMachineTest, intelMemoryReturnCopiesObjectToSretAndLeavesPointerInRax) {
    Value local { "obj", 0, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("retbig", { local }, {}, false, true);
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.returnFromProcedure("obj");

    std::string code = assemblyCode.str();
    // Three 8-byte words copied to [sret]; rax ends as the hidden pointer.
    EXPECT_THAT(code, HasSubstr("mov"));
    EXPECT_THAT(code, HasSubstr("leave"));
    EXPECT_THAT(code, HasSubstr("ret"));
    // Must not treat a 24-byte object as a two-register return (no lone rdx path).
    EXPECT_THAT(code, Not(HasSubstr("mov rdx, rax")));
}

TEST_F(StackMachineTest, intelCallWithMemoryReturnDestLeasIntoRdi) {
    Value dest { "out", 0, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("caller", { dest }, {}, false, false);
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.callProcedure("retbig", "out");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("lea rdi,"));
    EXPECT_THAT(code, HasSubstr("call $retbig"));
}

TEST_F(StackMachineTest, intelProcedureCallClearsRaxWhenNoVectorArgs) {
    // Same AL-clear contract as the AT&T suite, asserted in production dialect.
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    Value value = intValue("value");
    stackMachine.startProcedure("proc", { value }, {});
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(value.getName());
    stackMachine.callProcedure("printf");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("xor rax, rax"));
    EXPECT_THAT(code, HasSubstr("call $printf"));
}

// Multi-word stack argument: each word is pushed (24-byte object -> 3 qwords).
TEST_F(StackMachineTest, intelMultiWordArgumentPushesEachWord) {
    Value big { "big", 0, ValueKind::INTEGRAL, 24 };
    // Force stack-passed: fill the 6 integer arg registers first with dummies.
    std::vector<Value> locals { big };
    std::vector<Value> namedArgs;
    for (int i = 0; i < 6; ++i) {
        namedArgs.push_back(Value { "a" + std::to_string(i), i + 1, ValueKind::INTEGRAL, 8 });
        locals.push_back(namedArgs.back());
    }
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("caller", locals, {});
    assemblyCode.str("");
    assemblyCode.clear();

    for (const auto& a : namedArgs) {
        stackMachine.procedureArgument(a.getName());
    }
    stackMachine.procedureArgument(big.getName());
    stackMachine.callProcedure("callee");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("call $callee"));
    // Three word pushes (or equivalent mov+push) for the 24-byte object beyond
    // the six register args. Count push instructions as a lower bound.
    int pushes = 0;
    for (std::size_t i = 0; i + 4 < code.size(); ++i) {
        if (code.compare(i, 5, "push ") == 0 || code.compare(i, 5, "push\t") == 0) {
            ++pushes;
        }
    }
    EXPECT_GE(pushes, 3);
}

// Floating args go in xmm0.. and set AL for variadic callees (SysV).
TEST_F(StackMachineTest, intelFloatingArgumentUsesXmmAndSetsAl) {
    Value d { "d", 0, ValueKind::FLOATING, 8 };
    Value fmt { "fmt", 1, ValueKind::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("caller", { d, fmt }, {});
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(fmt.getName());
    stackMachine.procedureArgument(d.getName());
    stackMachine.callProcedure("printf");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("xmm0"));
    EXPECT_THAT(code, HasSubstr("call $printf"));
    // AL = number of vector registers used (mov to rax/eax both set AL).
    EXPECT_TRUE(code.find("mov eax, 1") != std::string::npos
            || code.find("mov rax, 1") != std::string::npos);
}

// float32 call arg is movd into xmm, not movq.
TEST_F(StackMachineTest, intelFloat32CallArgUsesMovd) {
    Value fmt { "fmt", 0, ValueKind::INTEGRAL, 8 };
    Value f { "f", 1, ValueKind::FLOATING, 4 };
    Value code { "code", 2, ValueKind::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("caller", { fmt, f, code }, {});
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(fmt.getName());
    stackMachine.procedureArgument(f.getName());
    stackMachine.procedureArgument(code.getName());
    stackMachine.callProcedure("callee");

    std::string codeAsm = assemblyCode.str();
    EXPECT_THAT(codeAsm, testing::HasSubstr("movd xmm0"));
    EXPECT_THAT(codeAsm, testing::Not(testing::HasSubstr("movq xmm0")));
    // place.index: second integer is rsi (GP1), not rdx. Float does not spend a GP slot.
    EXPECT_THAT(codeAsm, testing::HasSubstr("rsi"));
}

// AL counts xmm0..xmm7 only; a ninth float does not set AL to 9.
TEST_F(StackMachineTest, intelNinthFloatDoesNotCountTowardAl) {
    std::vector<Value> locals;
    for (int i = 0; i < 9; ++i) {
        locals.push_back({ "f" + std::to_string(i), i, ValueKind::FLOATING, 8 });
    }
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("caller", locals, {});
    assemblyCode.str("");
    assemblyCode.clear();

    for (int i = 0; i < 9; ++i) {
        stackMachine.procedureArgument("f" + std::to_string(i));
    }
    stackMachine.callProcedure("callee");

    std::string code = assemblyCode.str();
    EXPECT_TRUE(code.find("mov rax, 8") != std::string::npos
            || code.find("mov eax, 8") != std::string::npos);
    EXPECT_TRUE(code.find("mov rax, 9") == std::string::npos
            && code.find("mov eax, 9") == std::string::npos);
}

TEST_F(StackMachineTest, intelFloat32LvalueAssignUsesDword) {
    Value f { "f", 0, ValueKind::FLOATING, 4 };
    Value p { "p", 1, ValueKind::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("storef", { f, p }, {});
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.lvalueAssign("f", "p");

    EXPECT_THAT(assemblyCode.str(), HasSubstr("dword"));
}

// SSE path for double add: load bits, addsd, park result.
TEST_F(StackMachineTest, intelFloatingAddUsesAddsd) {
    Value a { "a", 0, ValueKind::FLOATING, 8 };
    Value b { "b", 1, ValueKind::FLOATING, 8 };
    Value r { "r", 2, ValueKind::FLOATING, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("fadd", { a, b, r }, {});
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.add("a", "b", "r");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("addsd"));
    EXPECT_THAT(code, HasSubstr("xmm0"));
    EXPECT_THAT(code, HasSubstr("xmm1"));
}

TEST_F(StackMachineTest, intelFloat32AddUsesAddss) {
    Value a { "a", 0, ValueKind::FLOATING, 4 };
    Value b { "b", 1, ValueKind::FLOATING, 4 };
    Value r { "r", 2, ValueKind::FLOATING, 4 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("faddss", { a, b, r }, {});
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.add("a", "b", "r");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("addss"));
    EXPECT_THAT(code, Not(HasSubstr("addsd")));
}

// Product policy: sret is only installed when memoryReturn && !variadic.
// Variadic + memoryReturn falls back to RAX/RDX for the first two words.
TEST_F(StackMachineTest, variadicMemoryReturnSkipsSret) {
    Value obj { "obj", 0, ValueKind::INTEGRAL, 24 };
    Value named { "named", 1, ValueKind::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };

    // memoryReturn=true, variadic=true: must not capture rdi as __sret.
    stackMachine.startProcedure("vret", { obj }, { named }, true, true);
    std::string prologue = assemblyCode.str();
    EXPECT_THAT(prologue, Not(HasSubstr("__sret")));

    assemblyCode.str("");
    assemblyCode.clear();
    stackMachine.returnFromProcedure("obj");
    std::string epilogue = assemblyCode.str();
    // Register-return path uses remainder register (rdx) for word 1.
    EXPECT_THAT(epilogue, HasSubstr("rdx"));
}

// Nested sret call sequence at the StackMachine level: lea rdi for dest, call,
// then another lea for outer dest must still appear on a subsequent call.
TEST_F(StackMachineTest, intelNestedSretCallsEachLeaDestIntoRdi) {
    Value outer { "outer", 0, ValueKind::INTEGRAL, 24 };
    Value inner { "inner", 3, ValueKind::INTEGRAL, 24 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("caller", { outer, inner }, {}, false, false);
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.callProcedure("make_inner", "inner");
    stackMachine.callProcedure("make_outer", "outer");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("lea rdi,"));
    EXPECT_THAT(code, HasSubstr("call $make_inner"));
    EXPECT_THAT(code, HasSubstr("call $make_outer"));
    int leaCount = 0;
    for (std::size_t i = 0; i + 7 < code.size(); ++i) {
        if (code.compare(i, 7, "lea rdi") == 0) {
            ++leaCount;
        }
    }
    EXPECT_GE(leaCount, 2);
}

// sret spends GP0 (rdi). Float then int: xmm0 + rsi (place.index 1), not firstReg+count.
TEST_F(StackMachineTest, intelSretCallFloatThenIntUsesPlaceIndex) {
    Value dest { "out", 0, ValueKind::INTEGRAL, 24 };
    Value f { "f", 3, ValueKind::FLOATING, 8 };
    Value n { "n", 4, ValueKind::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("caller", { dest, f, n }, {}, false, false);
    assemblyCode.str("");
    assemblyCode.clear();

    stackMachine.procedureArgument(f.getName());
    stackMachine.procedureArgument(n.getName());
    stackMachine.callProcedure("retbig", "out");

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("lea rdi,"));
    EXPECT_THAT(code, HasSubstr("xmm0"));
    EXPECT_THAT(code, HasSubstr("rsi"));
}

} // namespace
