#include "gtest/gtest.h"

#include "codegen/quadruples/Retrieve.h"

namespace {

// Retrieve carries an explicit sret/memory-return bit so StackMachine does not
// re-derive policy from size alone (must match Call's memoryReturnDest).
TEST(Retrieve, memoryReturnFlagDefaultsFalse) {
    codegen::Retrieve r { "ret0" };
    EXPECT_FALSE(r.isMemoryReturn());
    EXPECT_EQ(r.getResultName(), "ret0");
}

TEST(Retrieve, memoryReturnFlagExplicit) {
    codegen::Retrieve r { "ret1", true };
    EXPECT_TRUE(r.isMemoryReturn());
    EXPECT_EQ(r.getResultName(), "ret1");
}

} // namespace
