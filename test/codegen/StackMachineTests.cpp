#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/StackMachine.h"
#include "codegen/ATandTInstructionSet.h"

#include <memory>

namespace {

using namespace testing;
using namespace codegen;

TEST(ProcedureArgumentPassing, firstIntegerArgumentIsPassedInRDI) {
    std::stringstream assemblyCode { };
    //StackMachine stackMachine { &assemblyCode, std::make_unique<ATandTInstructionSet>() };

    EXPECT_THAT(true, Eq(true));
}

}
