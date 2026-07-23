#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/StackMachine.h"
#include "codegen/ATandTInstructionSet.h"
#include "codegen/IntelInstructionSet.h"

#include <memory>

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
        return { name, 0, Type::INTEGRAL, 8 };
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

    Value v1 { "v1", 0, Type::INTEGRAL, 8 };
    Value v2 { "v2", 1, Type::INTEGRAL, 8 };
    Value v3 { "v3", 2, Type::INTEGRAL, 8 };

private:

};

TEST_F(StackMachineTest, procedureCall_doesNotPushUnusedCallerSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };

    stackMachine.callProcedure("procedure");

    expectCode("\txorq %rax, %rax\n"
            "\tcall procedure\n");
}

TEST_F(StackMachineTest, procedureCall_storesAllDirtyCallerSavedRegisters) {
    Value value { "value", 0, Type::INTEGRAL, 4 };
    for (auto &reg : registers->getGeneralPurposeRegisters()) {
        reg->assign(&value);
    }
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };

    stackMachine.callProcedure("procedure");

    expectCode("\tmovq %rax, (%rsp)\n"
            "\tmovq %rcx, (%rsp)\n"
            "\tmovq %rdx, (%rsp)\n"
            "\tmovq %rsi, (%rsp)\n"
            "\tmovq %rdi, (%rsp)\n"
            "\tmovq %r8, (%rsp)\n"
            "\tmovq %r9, (%rsp)\n"
            "\tmovq %r10, (%rsp)\n"
            "\tmovq %r11, (%rsp)\n"
            "\txorq %rax, %rax\n"
            "\tcall procedure\n");
}

// Variadic ABI: AL must be 0 when no vector args are passed (e.g. printf with integers only)
TEST_F(StackMachineTest, procedureCall_clearsRaxForVariadicAlRequirement) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value value = intValue("value");
    stackMachine.startProcedure("proc", { value }, { });
    assemblyCode.str("");

    stackMachine.procedureArgument(value.getName());
    stackMachine.callProcedure("printf");

    // Register args are loaded, then rematerialized after spillCallerSavedRegisters
    // (arg regs use loadWithoutBinding so spill may leave values only in memory).
    expectCode("\tmovq -40(%rsp), %rdi\n"
            "\tmovq -40(%rsp), %rdi\n"
            "\txorq %rax, %rax\n"
            "\tcall printf\n");
}

TEST_F(StackMachineTest, procedureStart_storesCalleeSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };

    stackMachine.startProcedure("proc", { }, { });

    expectCode("proc:\n"
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
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };

    stackMachine.returnFromProcedure();

    expectCode("\tleave\n"
            "\tret\n");
}

TEST_F(StackMachineTest, procedureReturn_popsCalleeSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };
    stackMachine.startProcedure("proc", { }, { });
    assemblyCode.str("");

    stackMachine.returnFromProcedure();

    expectCode("\tpopq %r15\n"
            "\tpopq %r14\n"
            "\tpopq %r13\n"
            "\tpopq %r12\n"
            "\tpopq %rbx\n"
            "\tleave\n"
            "\tret\n");
}

TEST_F(StackMachineTest, procedureArgumentPassing_firstIntegerArgumentIsPassedInRDI) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value value = intValue("value");
    stackMachine.startProcedure("proc", { value }, { });
    assemblyCode.str("");

    stackMachine.procedureArgument(value.getName());
    stackMachine.callProcedure("procedure");

    expectCode("\tmovq -40(%rsp), %rdi\n"
            "\tmovq -40(%rsp), %rdi\n"
            "\txorq %rax, %rax\n"
            "\tcall procedure\n");
}

// Odd number of stack args needs 8-byte padding so RSP is 16-byte aligned before call
TEST_F(StackMachineTest, procedureCall_padsStackForOddNumberOfStackArguments) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>() };
    std::vector<Value> locals;
    for (int i = 0; i < 7; ++i) {
        locals.push_back({ "a" + std::to_string(i), i, Type::INTEGRAL, 8 });
    }
    stackMachine.startProcedure("proc", locals, { });
    assemblyCode.str("");

    for (const auto& local : locals) {
        stackMachine.procedureArgument(local.getName());
    }
    stackMachine.callProcedure("procedure");

    // Integer args loaded twice: once before spill, rematerialized after.
    expectCode("\tmovq -40(%rsp), %rdi\n"
            "\tmovq -48(%rsp), %rsi\n"
            "\tmovq -56(%rsp), %rdx\n"
            "\tmovq -64(%rsp), %rcx\n"
            "\tmovq -72(%rsp), %r8\n"
            "\tmovq -80(%rsp), %r9\n"
            "\tmovq -40(%rsp), %rdi\n"
            "\tmovq -48(%rsp), %rsi\n"
            "\tmovq -56(%rsp), %rdx\n"
            "\tmovq -64(%rsp), %rcx\n"
            "\tmovq -72(%rsp), %r8\n"
            "\tmovq -80(%rsp), %r9\n"
            "\tsubq $8, %rsp\n"
            "\tmovq -96(%rsp), %rax\n"
            "\tpushq %rax\n"
            "\txorq %rax, %rax\n"
            "\tcall procedure\n"
            "\taddq $16, %rsp\n");
}

TEST_F(StackMachineTest, sub_reg_reg) {
    // given
    rax->assign(&v1);
    rbx->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

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

    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmovq %rax, %rbx\n"
            "\tsubq -8(%rsp), %rbx\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, sub_mem_reg) {
    // given
    rax->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmovq (%rsp), %rbx\n"
            "\tsubq %rax, %rbx\n");

    expectRegisterContains(rax, v2);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, sub_mem_mem) {
    // given
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.sub(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmovq (%rsp), %rax\n"
            "\tsubq -8(%rsp), %rax\n");

    expectRegisterContains(rax, v3);
}

TEST_F(StackMachineTest, add_reg_reg) {
    // given
    rax->assign(&v1);
    rbx->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

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

    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmovq %rax, %rbx\n"
            "\taddq -8(%rsp), %rbx\n");

    expectRegisterContains(rax, v1);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, add_mem_reg) {
    // given
    rax->assign(&v2);

    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmovq (%rsp), %rbx\n"
            "\taddq %rax, %rbx\n");

    expectRegisterContains(rax, v2);
    expectRegisterContains(rbx, v3);
}

TEST_F(StackMachineTest, add_mem_mem) {
    // given
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };
    stackMachine.setScope( { v1, v2, v3 });

    // when
    stackMachine.add(v1.getName(), v2.getName(), v3.getName());

    // then
    expectCode("\tmovq (%rsp), %rax\n"
            "\taddq -8(%rsp), %rax\n");

    expectRegisterContains(rax, v3);
}



}

TEST_F(StackMachineTest, variadicPrologueCallsTransVaSetAreas) {
    // Production path uses Intel/NASM instruction set (LEA for save-area base).
    Value arg { "last", 0, Type::INTEGRAL, 8, true };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };

    stackMachine.startProcedure("varfn", {}, { arg }, true, false);

    std::string code = assemblyCode.str();
    EXPECT_THAT(code, HasSubstr("__trans_va_set_areas"));
    EXPECT_THAT(code, HasSubstr("mov"));
}

TEST_F(StackMachineTest, nonVariadicPrologueDoesNotCallTransVaSetAreas) {
    Value arg { "x", 0, Type::INTEGRAL, 8, true };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("plain", {}, { arg }, false, false);
    EXPECT_THAT(assemblyCode.str(), Not(HasSubstr("__trans_va_set_areas")));
}

TEST_F(StackMachineTest, variadicReturnPopsVaAreas) {
    Value arg { "last", 0, Type::INTEGRAL, 8, true };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };
    stackMachine.startProcedure("varfn", {}, { arg }, true, false);
    assemblyCode.str("");
    assemblyCode.clear();
    stackMachine.returnFromProcedure("");
    EXPECT_THAT(assemblyCode.str(), HasSubstr("__trans_va_pop_areas"));
}

// --- Production Intel/NASM path: System V memory return (sret) ---

TEST_F(StackMachineTest, intelMemoryReturnPrologueHoldsSretInFirstArgReg) {
    // Aggregates > 16 bytes: callee receives hidden pointer in rdi (first integer arg).
    Value local { "obj", 0, Type::INTEGRAL, 24 };
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
    Value local { "obj", 0, Type::INTEGRAL, 24 };
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
    Value dest { "out", 0, Type::INTEGRAL, 24 };
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
    Value big { "big", 0, Type::INTEGRAL, 24 };
    // Force stack-passed: fill the 6 integer arg registers first with dummies.
    std::vector<Value> locals { big };
    std::vector<Value> namedArgs;
    for (int i = 0; i < 6; ++i) {
        namedArgs.push_back(Value { "a" + std::to_string(i), i + 1, Type::INTEGRAL, 8 });
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
    Value d { "d", 0, Type::FLOATING, 8 };
    Value fmt { "fmt", 1, Type::INTEGRAL, 8 };
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
    // AL = number of vector registers used (at least mov eax, 1).
    EXPECT_THAT(code, HasSubstr("mov eax, 1"));
}

// Product policy: sret is only installed when memoryReturn && !variadic.
// Variadic + memoryReturn falls back to RAX/RDX for the first two words.
TEST_F(StackMachineTest, variadicMemoryReturnSkipsSret) {
    Value obj { "obj", 0, Type::INTEGRAL, 24 };
    Value named { "named", 1, Type::INTEGRAL, 8 };
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(),
            std::make_unique<Amd64Registers>() };

    // memoryReturn=true, variadic=true: must not capture rdi as __sret.
    stackMachine.startProcedure("vret", { obj }, { named }, true, true);
    std::string prologue = assemblyCode.str();
    EXPECT_THAT(prologue, HasSubstr("__trans_va_set_areas"));
    EXPECT_THAT(prologue, Not(HasSubstr("__sret")));

    assemblyCode.str("");
    assemblyCode.clear();
    stackMachine.returnFromProcedure("obj");
    std::string epilogue = assemblyCode.str();
    // Register-return path uses remainder register (rdx) for word 1.
    EXPECT_THAT(epilogue, HasSubstr("rdx"));
    EXPECT_THAT(epilogue, HasSubstr("__trans_va_pop_areas"));
}

// Nested sret call sequence at the StackMachine level: lea rdi for dest, call,
// then another lea for outer dest must still appear on a subsequent call.
TEST_F(StackMachineTest, intelNestedSretCallsEachLeaDestIntoRdi) {
    Value outer { "outer", 0, Type::INTEGRAL, 24 };
    Value inner { "inner", 3, Type::INTEGRAL, 24 };
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
