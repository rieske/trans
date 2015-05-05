#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/StackMachine.h"
#include "codegen/ATandTInstructionSet.h"

#include <memory>

namespace {

using testing::Eq;
using testing::StrEq;
using namespace codegen;

TEST(ProcedureCall, storesCallerSavedRegisters) {
    std::stringstream assemblyCode { };
    std::unique_ptr<Amd64Registers> registers = std::make_unique<Amd64Registers>();
    Value value { "value", 0, Type::INTEGRAL, 4 };
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        reg->assign(&value);
    }
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::move(registers) };

    stackMachine.callProcedure("procedure");

    EXPECT_THAT(assemblyCode.str(),
            StrEq("\tmovq %rax, -40(%rsp)\n"
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
                    "\tpopq %rcx\n"));

}

TEST(ProcedureArgumentPassing, firstIntegerArgumentIsPassedInRDI) {
    std::stringstream assemblyCode { };
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>() };

    stackMachine.procedureArgument("a");

    EXPECT_THAT(assemblyCode.str(), Eq(""));
}

}
