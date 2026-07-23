#include "gtest/gtest.h"

#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "symbols/FunctionEntry.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

// symbols is a leaf layer: no dependency on ast or semantic_analyzer.
// These tests pin the public surface after the layer split.

namespace {

translation_unit::Context ctx() { return { "t", 1 }; }

TEST(Symbols, valueEntryHoldsTypeAndName) {
    symbols::ValueEntry v { "x", type::signedInteger(), false, ctx(), 0 };
    EXPECT_EQ(v.getName(), "x");
    EXPECT_TRUE(v.getType().isPrimitive());
    EXPECT_EQ(v.getIndex(), 0);
}

TEST(Symbols, labelEntryName) {
    symbols::LabelEntry lab { "L1" };
    EXPECT_EQ(lab.getName(), "L1");
}

TEST(Symbols, functionEntryArity) {
    auto fty = type::function(type::signedInteger(), { type::signedInteger() });
    symbols::FunctionEntry f { "f", fty.getFunction(), ctx() };
    EXPECT_EQ(f.getName(), "f");
    EXPECT_EQ(f.argumentCount(), 1u);
}

} // namespace
