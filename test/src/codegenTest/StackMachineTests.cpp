#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/StackMachine.h"
#include "codegen/ATandTInstructionSet.h"

#include <memory>

namespace {

using testing::Eq;
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

    expectCode("\tcall procedure\n");
}

TEST_F(StackMachineTest, procedureCall_storesAllDirtyCallerSavedRegisters) {
    Value value { "value", 0, Type::INTEGRAL, 4 };
    for (auto &reg : registers->getGeneralPurposeRegisters()) {
        reg->assign(&value);
    }
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };

    stackMachine.callProcedure("procedure");

    expectCode("\tmovq %rax, (%rsp)\n"
            "\tpushq %rcx\n"
            "\tpushq %rdx\n"
            "\tpushq %rsi\n"
            "\tpushq %rdi\n"
            "\tpushq %r8\n"
            "\tpushq %r9\n"
            "\tpushq %r10\n"
            "\tpushq %r11\n"
            "\tcall procedure\n"
            "\tpopq %r11\n"
            "\tpopq %r10\n"
            "\tpopq %r9\n"
            "\tpopq %r8\n"
            "\tpopq %rdi\n"
            "\tpopq %rsi\n"
            "\tpopq %rdx\n"
            "\tpopq %rcx\n");
}

TEST_F(StackMachineTest, procedureStart_storesCalleeSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };

    stackMachine.startProcedure("proc", { }, { });

    expectCode("proc:\n"
            "\tpushq %rbp\n"
            "\tmovq %rsp, %rbp\n"
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
            "\tcall procedure\n");
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



}
