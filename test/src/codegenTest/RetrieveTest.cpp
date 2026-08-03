#include "gtest/gtest.h"

#include "codegen/Instruction.h"

namespace {

// Retrieve carries an explicit sret/memory-return bit so StackMachine does not
// re-derive policy from size alone (must match Call's memoryReturnDest).
TEST(Retrieve, memoryReturnFlagDefaultsFalse) {
    codegen::Instruction r = codegen::ir::retrieve("ret0");
    EXPECT_FALSE(r.memoryReturn);
    EXPECT_EQ(r.result, "ret0");
}

TEST(Retrieve, memoryReturnFlagExplicit) {
    codegen::Instruction r = codegen::ir::retrieve("ret1", true);
    EXPECT_TRUE(r.memoryReturn);
    EXPECT_EQ(r.result, "ret1");
}

} // namespace
