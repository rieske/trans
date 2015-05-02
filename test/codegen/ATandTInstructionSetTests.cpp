#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/ATandTInstructionSet.h"

#include <memory>

namespace {

using namespace testing;
using namespace codegen;

TEST(ATandTInstructionSet, emitsPreamble) {
    ATandTInstructionSet instructions;

    EXPECT_THAT(instructions.preamble(), Eq(".extern scanf\n"
            ".extern printf\n\n"
            ".data\n"
            "sfmt: .string \"%d\"\n"
            "fmt: .string \"%d\n\"\n\n"
            ".text\n"
            ".globl main\n\n"));
}

}
