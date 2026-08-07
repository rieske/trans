#include "gtest/gtest.h"

#include "codegen/Instruction.h"
#include "codegen/SymbolRefs.h"

#include <algorithm>

namespace {

using namespace codegen;

// SymbolRefs must expose uses/defs explicitly (no "names bag" + inference).

TEST(SymbolRefs, addressOfReportsUseDefAndBase) {
    Instruction op = ir::addressOf("$t1", "$t2");
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    ASSERT_EQ(refs.uses.size(), 1u);
    EXPECT_EQ(refs.uses[0], "$t1");
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], "$t2");
    EXPECT_EQ(refs.addressOfBase, "$t1");
    EXPECT_FALSE(refs.isParam);
    EXPECT_FALSE(refs.isCall);
}

TEST(SymbolRefs, functionAddressOnlyDefsResultNotAddressOfBase) {
    Instruction op = ir::functionAddress("foo", "$t9");
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    EXPECT_TRUE(refs.uses.empty());
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], "$t9");
    EXPECT_TRUE(refs.addressOfBase.empty());
}

// Pool labels (string literals) define a result temp. Without this def, frame
// packing starts the live range at the first PARAM and reuses its spill slot
// for nested-call args that are still live when the label address is spilled.
TEST(SymbolRefs, assignLabelAddressDefsResult) {
    Instruction op = ir::assignLabelAddress("L$str1", "$t6");
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    EXPECT_TRUE(refs.uses.empty());
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], "$t6");
    EXPECT_TRUE(refs.addressOfBase.empty());
    EXPECT_FALSE(refs.isParam);
    EXPECT_FALSE(refs.isCall);
}

TEST(SymbolRefs, argumentIsParamUse) {
    Instruction op = ir::argument("$t3");
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    EXPECT_TRUE(refs.isParam);
    ASSERT_EQ(refs.uses.size(), 1u);
    EXPECT_EQ(refs.uses[0], "$t3");
    EXPECT_TRUE(refs.defs.empty());
}

TEST(SymbolRefs, callIsCallIndirectUsesTarget) {
    Instruction direct = ir::call("foo", false, "");
    SymbolRefs d;
    collectSymbolRefs(direct, d);
    EXPECT_TRUE(d.isCall);
    EXPECT_TRUE(d.uses.empty());

    Instruction indirect = ir::call("$t_fn", true, "$t_sret");
    SymbolRefs i;
    collectSymbolRefs(indirect, i);
    EXPECT_TRUE(i.isCall);
    EXPECT_NE(std::find(i.uses.begin(), i.uses.end(), "$t_fn"), i.uses.end());
    EXPECT_NE(std::find(i.uses.begin(), i.uses.end(), "$t_sret"), i.uses.end());
}

TEST(SymbolRefs, binaryAddUsesOperandsDefsResult) {
    Instruction op = ir::add("$t1", "$t2", "$t3");
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    ASSERT_EQ(refs.uses.size(), 2u);
    EXPECT_EQ(refs.uses[0], "$t1");
    EXPECT_EQ(refs.uses[1], "$t2");
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], "$t3");
}

TEST(SymbolRefs, assignUsesOperandDefsResult) {
    Instruction op = ir::assign("$t1", "$t2");
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    ASSERT_EQ(refs.uses.size(), 1u);
    EXPECT_EQ(refs.uses[0], "$t1");
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], "$t2");
    EXPECT_TRUE(refs.addressOfBase.empty());
}

} // namespace
