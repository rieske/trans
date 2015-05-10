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
            registers { std::make_unique<Amd64Registers>() }
    {
    }

protected:
    void expectCode(std::string expectedCode) {
        EXPECT_THAT(assemblyCode.str(), StrEq(expectedCode));
    }

    std::unique_ptr<Amd64Registers> registers;
    std::stringstream assemblyCode { };

private:

};

TEST_F(StackMachineTest, procedureCall_doesNotPushUnusedCallerSavedRegisters) {
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };

    stackMachine.callProcedure("procedure");

    expectCode("\tcall procedure\n");
}

TEST_F(StackMachineTest, procedureCall_storesAllDirtyCallerSavedRegisters) {
    Value value { "value", 0, Type::INTEGRAL, 4 };
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
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
    Value value { "value", 0, Type::INTEGRAL, 8 };
    stackMachine.startProcedure("proc", { value }, { });
    assemblyCode.str("");

    stackMachine.procedureArgument("value");
    stackMachine.callProcedure("procedure");

    expectCode("\tmovq -40(%rsp), %rdi\n"
            "\tcall procedure\n");
}

}
