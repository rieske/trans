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
            StrEq("\tmovq %rax, (%rsp)\n"
                    "\tmovq %rcx, (%rsp)\n"
                    "\tmovq %rdx, (%rsp)\n"
                    "\tmovq %rsi, (%rsp)\n"
                    "\tmovq %rdi, (%rsp)\n"
                    "\tmovq %r8, (%rsp)\n"
                    "\tmovq %r9, (%rsp)\n"
                    "\tmovq %r10, (%rsp)\n"
                    "\tmovq %r11, (%rsp)\n"
                    "\tcall procedure\n"));
}

TEST(ProcedureArgumentPassing, firstIntegerArgumentIsPassedInRDI) {
    std::stringstream assemblyCode { };
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>() };

    stackMachine.procedureArgument("a");

    EXPECT_THAT(assemblyCode.str(), Eq(""));
}

}
