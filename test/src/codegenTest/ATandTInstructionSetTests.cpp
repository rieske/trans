#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/ATandTInstructionSet.h"
#include "codegen/Register.h"

#include <memory>

namespace {

using namespace testing;
using namespace codegen;

ATandTInstructionSet instructions;

TEST(ATandTInstructionSet, emitsPreamble) {
    EXPECT_THAT(instructions.preamble(), Eq(".extern scanf\n"
            ".extern printf\n\n"
            ".data\n"
            "sfmt: .string \"%d\"\n"
            "fmt: .string \"%d\n\"\n\n"
            ".text\n"
            ".globl main\n\n"));
}

TEST(ATandTInstructionSet, emitsMovToMemoryWithOffset) {
    Register source { "src" };
    Register memoryBase { "memBase" };
    EXPECT_THAT(instructions.mov(source, memoryBase, 42), Eq("movq %src, -42(%memBase)"));
}

TEST(ATandTInstructionSet, emitsMovToMemoryWithoutOffset) {
    Register source { "src" };
    Register memoryBase { "memBase" };
    EXPECT_THAT(instructions.mov(source, memoryBase, 0), Eq("movq %src, (%memBase)"));
}

TEST(ATandTInstructionSet, emitsQuadSubtract) {
    Register reg { "reg" };
    EXPECT_THAT(instructions.sub(reg, 42), Eq("subq %reg, $42"));
}

}
