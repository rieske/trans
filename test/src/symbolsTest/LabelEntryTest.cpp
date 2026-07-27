#include "gtest/gtest.h"

#include "symbols/LabelEntry.h"

namespace {

TEST(LabelEntry, name) {
    symbols::LabelEntry lab { "L1" };
    EXPECT_EQ(lab.getName(), "L1");
}

} // namespace
