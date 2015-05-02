#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/StackMachine.h"
#include "codegen/ATandTInstructionSet.h"

#include <memory>

namespace {

using namespace testing;
using namespace codegen;

TEST(ProcedureCall, storesCallerSavedRegisters) {
    std::stringstream assemblyCode { };
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>() };

    stackMachine.callProcedure("procedure");

    EXPECT_THAT(assemblyCode.str(), Eq("\tcall procedure\n"));
}

TEST(ProcedureArgumentPassing, firstIntegerArgumentIsPassedInRDI) {
    std::stringstream assemblyCode { };
    StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>(), std::make_unique<Amd64Registers>() };

    stackMachine.procedureArgument("a");

    EXPECT_THAT(assemblyCode.str(), Eq(""));
}

}
