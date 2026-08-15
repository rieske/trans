#include "gtest/gtest.h"

#include "codegen/Instruction.h"
#include "codegen/SymbolRefs.h"

namespace {

using namespace codegen;

TEST(SymbolRefs, addressOfReportsUseDefAndBase) {
    IrStringTable strings;
    const int t1 = strings.intern("$t1");
    const int t2 = strings.intern("$t2");
    Instruction op = ir::addressOf(t1, t2);
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    ASSERT_EQ(refs.uses.size(), 1u);
    EXPECT_EQ(refs.uses[0], t1);
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], t2);
    EXPECT_EQ(refs.addressOfBase, t1);
    EXPECT_FALSE(refs.isParam);
    EXPECT_FALSE(refs.isCall);
}

TEST(SymbolRefs, leaObjectFieldAddressReportsAddressOfBase) {
    IrStringTable strings;
    const int t1 = strings.intern("$t1");
    const int t2 = strings.intern("$t2");
    Instruction op = ir::fieldAddress(t1, 8, t2, symbols::AddressBaseMode::LeaObject);
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    EXPECT_EQ(refs.addressOfBase, t1);
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], t2);
}

TEST(SymbolRefs, pointerValueFieldAddressDoesNotReportAddressOfBase) {
    IrStringTable strings;
    const int t1 = strings.intern("$t1");
    const int t2 = strings.intern("$t2");
    Instruction op = ir::fieldAddress(t1, 8, t2, symbols::AddressBaseMode::PointerValue);
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    EXPECT_EQ(refs.addressOfBase, kNoSymbol);
}

TEST(SymbolRefs, functionAddressOnlyDefsResultNotAddressOfBase) {
    IrStringTable strings;
    const int foo = strings.intern("foo");
    const int t9 = strings.intern("$t9");
    Instruction op = ir::functionAddress(foo, t9);
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    EXPECT_TRUE(refs.uses.empty());
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], t9);
    EXPECT_EQ(refs.addressOfBase, kNoSymbol);
}

TEST(SymbolRefs, assignLabelAddressDefsResult) {
    IrStringTable strings;
    const int str = strings.intern("__str1");
    const int t6 = strings.intern("$t6");
    Instruction op = ir::assignLabelAddress(str, t6);
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    EXPECT_TRUE(refs.uses.empty());
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], t6);
    EXPECT_EQ(refs.addressOfBase, kNoSymbol);
    EXPECT_FALSE(refs.isParam);
    EXPECT_FALSE(refs.isCall);
}

TEST(SymbolRefs, assignIsUseDefNotAddressOf) {
    IrStringTable strings;
    const int t1 = strings.intern("$t1");
    const int t2 = strings.intern("$t2");
    Instruction op = ir::assign(t1, t2);
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    ASSERT_EQ(refs.uses.size(), 1u);
    EXPECT_EQ(refs.uses[0], t1);
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], t2);
    EXPECT_EQ(refs.addressOfBase, kNoSymbol);
}

TEST(SymbolRefs, argumentIsParamUse) {
    IrStringTable strings;
    const int t3 = strings.intern("$t3");
    Instruction op = ir::argument(t3);
    SymbolRefs refs;
    collectSymbolRefs(op, refs);
    EXPECT_TRUE(refs.isParam);
    ASSERT_EQ(refs.uses.size(), 1u);
    EXPECT_EQ(refs.uses[0], t3);
    EXPECT_TRUE(refs.defs.empty());
}

TEST(SymbolRefs, callIsCallIndirectUsesTarget) {
    IrStringTable strings;
    const int foo = strings.intern("foo");
    const int target = strings.intern("$fn");
    Instruction direct = ir::call(foo, false, kNoSymbol);
    SymbolRefs d;
    collectSymbolRefs(direct, d);
    EXPECT_TRUE(d.isCall);
    EXPECT_TRUE(d.uses.empty());

    Instruction indirect = ir::call(target, true, kNoSymbol);
    SymbolRefs n;
    collectSymbolRefs(indirect, n);
    EXPECT_TRUE(n.isCall);
    ASSERT_EQ(n.uses.size(), 1u);
    EXPECT_EQ(n.uses[0], target);
}

} // namespace
