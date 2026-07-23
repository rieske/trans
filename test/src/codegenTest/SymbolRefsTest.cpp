#include "gtest/gtest.h"

#include "codegen/quadruples/AddressOf.h"
#include "codegen/quadruples/Argument.h"
#include "codegen/quadruples/Assign.h"
#include "codegen/quadruples/Call.h"
#include "codegen/quadruples/DoubleOperandQuadruple.h"
#include "codegen/quadruples/FunctionAddress.h"
#include "codegen/quadruples/Add.h"
#include "codegen/quadruples/Quadruple.h"

#include <algorithm>

namespace {

using namespace codegen;

// SymbolRefs must expose uses/defs explicitly (no "names bag" + inference).

TEST(SymbolRefs, addressOfReportsUseDefAndBase) {
    AddressOf op("$t1", "$t2");
    SymbolRefs refs;
    op.collectSymbolRefs(refs);
    ASSERT_EQ(refs.uses.size(), 1u);
    EXPECT_EQ(refs.uses[0], "$t1");
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], "$t2");
    EXPECT_EQ(refs.addressOfBase, "$t1");
    EXPECT_FALSE(refs.isParam);
    EXPECT_FALSE(refs.isCall);
}

TEST(SymbolRefs, functionAddressOnlyDefsResultNotAddressOfBase) {
    FunctionAddress op("foo", "$t9");
    SymbolRefs refs;
    op.collectSymbolRefs(refs);
    EXPECT_TRUE(refs.uses.empty());
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], "$t9");
    EXPECT_TRUE(refs.addressOfBase.empty());
}

TEST(SymbolRefs, argumentIsParamUse) {
    Argument op("$t3");
    SymbolRefs refs;
    op.collectSymbolRefs(refs);
    EXPECT_TRUE(refs.isParam);
    ASSERT_EQ(refs.uses.size(), 1u);
    EXPECT_EQ(refs.uses[0], "$t3");
    EXPECT_TRUE(refs.defs.empty());
}

TEST(SymbolRefs, callIsCallIndirectUsesTarget) {
    Call direct("foo", false, "");
    SymbolRefs d;
    direct.collectSymbolRefs(d);
    EXPECT_TRUE(d.isCall);
    EXPECT_TRUE(d.uses.empty());

    Call indirect("$t_fn", true, "$t_sret");
    SymbolRefs i;
    indirect.collectSymbolRefs(i);
    EXPECT_TRUE(i.isCall);
    EXPECT_NE(std::find(i.uses.begin(), i.uses.end(), "$t_fn"), i.uses.end());
    EXPECT_NE(std::find(i.uses.begin(), i.uses.end(), "$t_sret"), i.uses.end());
}

TEST(SymbolRefs, binaryAddUsesOperandsDefsResult) {
    Add op("$t1", "$t2", "$t3");
    SymbolRefs refs;
    op.collectSymbolRefs(refs);
    ASSERT_EQ(refs.uses.size(), 2u);
    EXPECT_EQ(refs.uses[0], "$t1");
    EXPECT_EQ(refs.uses[1], "$t2");
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], "$t3");
}

TEST(SymbolRefs, assignUsesOperandDefsResult) {
    Assign op("$t1", "$t2");
    SymbolRefs refs;
    op.collectSymbolRefs(refs);
    ASSERT_EQ(refs.uses.size(), 1u);
    EXPECT_EQ(refs.uses[0], "$t1");
    ASSERT_EQ(refs.defs.size(), 1u);
    EXPECT_EQ(refs.defs[0], "$t2");
    EXPECT_TRUE(refs.addressOfBase.empty());
}

} // namespace
