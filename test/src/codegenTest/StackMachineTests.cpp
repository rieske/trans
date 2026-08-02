#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/StackMachine.h"
#include "codegen/ATandTInstructionSet.h"
#include "codegen/IntelInstructionSet.h"

#include <memory>

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
            "\tcall procedure@plt\n");
}

// Production path uses IntelInstructionSet (LEA + indirect call via r10).
TEST_F(StackMachineTest, functionAddress_leaDefinedProcedure) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value fp = intValue("fp");
    stackMachine.registerDefinedProcedure("foo");
    stackMachine.startProcedure("foo", { fp }, { });
    assemblyCode.str("");

    stackMachine.functionAddress("foo", "fp");

    // Same-TU definition: PC-relative LEA (PIE-safe).
    expectCode("\tlea rax, [rel foo]\n");
}

TEST_F(StackMachineTest, functionAddress_loadsExternViaGot) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value fp = intValue("fp");
    stackMachine.registerDefinedProcedure("proc");
    stackMachine.startProcedure("proc", { fp }, { });
    assemblyCode.str("");

    stackMachine.functionAddress("printf", "fp");

    // Extern: GOT load for PIE.
    expectCode("\tmov rax, [rel printf wrt ..got]\n");
}

// Pool/data labels (string constants): same lea [rel] + bindResult as defined functionAddress.
TEST_F(StackMachineTest, assignLabelAddress_leaPoolLabel) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value s = intValue("s");
    stackMachine.startProcedure("proc", { s }, { });
    assemblyCode.str("");

    stackMachine.assignLabelAddress("L$str1", "s");

    expectCode("\tlea rax, [rel L$str1]\n");
}

TEST_F(StackMachineTest, callProcedure_sameTuDoesNotUsePlt) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    stackMachine.registerDefinedProcedure("foo");

    stackMachine.callProcedure("foo");

    // NASM rejects local `call foo wrt ..plt` (intra-segment OUT_REL4ADR).
    expectCode("\txor rax, rax\n"
            "\tcall foo\n");
}

TEST_F(StackMachineTest, callProcedure_externUsesPlt) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };

    stackMachine.callProcedure("printf");

    expectCode("\txor rax, rax\n"
            "\tcall printf wrt ..plt\n");
}

// Target already in a callee-saved reg survives spillCallerSavedRegisters → mov to r10.
TEST_F(StackMachineTest, callProcedureIndirect_movesRegisterTargetToR10) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value junk = intValue("junk");
    Value fp = { "fp", 1, Type::INTEGRAL, 8 };
    stackMachine.startProcedure("proc", { junk, fp }, { });
    // Occupy rax so the next functionAddress binds fp to rbx (callee-saved).
    stackMachine.functionAddress("a", "junk");
    stackMachine.functionAddress("foo", "fp");
    assemblyCode.str("");

    stackMachine.callProcedureIndirect("fp");

    // Spill junk from rax (retrieval reg), then mov callee-saved fp → r10 and call.
    expectCode("\tmov [rsp + 40], rax\n"
            "\txor rax, rax\n"
            "\tmov r10, rbx\n"
            "\tcall r10\n");
}

TEST_F(StackMachineTest, callProcedureIndirect_loadsMemoryTargetToR10) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<IntelInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value fp = intValue("fp");
    stackMachine.startProcedure("proc", { fp }, { });
    assemblyCode.str("");

    stackMachine.callProcedureIndirect("fp");

    // Local starts in a frame slot; load into r10 then indirect call.
    EXPECT_THAT(assemblyCode.str(), testing::HasSubstr("call r10"));
    EXPECT_THAT(assemblyCode.str(), testing::HasSubstr("r10"));
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
            "\tcall procedure@plt\n");
}

// Variadic ABI: AL must be 0 when no vector args are passed (e.g. printf with integers only)
TEST_F(StackMachineTest, procedureCall_clearsRaxForVariadicAlRequirement) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>() };
    Value value = intValue("value");
    stackMachine.startProcedure("proc", { value }, { });
    assemblyCode.str("");

    stackMachine.procedureArgument(value.getName());
    stackMachine.callProcedure("printf");

    expectCode("\tmovq 40(%rsp), %rdi\n"
            "\txorq %rax, %rax\n"
            "\tcall printf@plt\n");
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

    expectCode("\tmovq 40(%rsp), %rdi\n"
            "\txorq %rax, %rax\n"
            "\tcall procedure@plt\n");
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

    expectCode("\tmovq 40(%rsp), %rdi\n"
            "\tmovq 48(%rsp), %rsi\n"
            "\tmovq 56(%rsp), %rdx\n"
            "\tmovq 64(%rsp), %rcx\n"
            "\tmovq 72(%rsp), %r8\n"
            "\tmovq 80(%rsp), %r9\n"
            "\tsubq $8, %rsp\n"
            "\tmovq 96(%rsp), %rax\n"
            "\tpushq %rax\n"
            "\txorq %rax, %rax\n"
            "\tcall procedure@plt\n"
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
            "\tsubq 8(%rsp), %rbx\n");

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
            "\tsubq 8(%rsp), %rax\n");

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
            "\taddq 8(%rsp), %rbx\n");

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
            "\taddq 8(%rsp), %rax\n");

    expectRegisterContains(rax, v3);
}



}
