#include "gtest/gtest.h"

#include "ast/IdentifierExpression.h"
#include "symbols/AnnotationStore.h"
#include "symbols/ValueEntry.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

namespace {

translation_unit::Context ctx() {
    return { "t", 1 };
}

TEST(Expression, expressionTypeThrowsWhenUnset) {
    ast::IdentifierExpression id("x", ctx());
    EXPECT_THROW(id.expressionType(), std::runtime_error);
}

TEST(Expression, valueTypeFallsBackToExpressionType) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression id("x", ctx());
    id.setType(type::signedInteger());
    EXPECT_TRUE(id.valueType(store).isPrimitive());
    EXPECT_FALSE(id.hasResultSymbol(store));
}

TEST(Expression, hasDecayedArrayValue) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression id("a", ctx());
    type::Type arr = type::array(type::signedInteger(), 3);
    symbols::ValueEntry addr("t", type::pointer(type::signedInteger()), true, ctx(), 0);
    id.setAggregateAddressResult(store, addr, arr);
    EXPECT_TRUE(id.holdsAggregateAddress());
    EXPECT_TRUE(id.isArrayObjectType());
    EXPECT_TRUE(id.hasDecayedArrayValue(store));
    EXPECT_TRUE(store.hasResult(&id));
}

TEST(Expression, setTypeAndResultIsScalar) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression id("x", ctx());
    symbols::ValueEntry v("x", type::signedInteger(), false, ctx(), 0);
    id.setTypeAndResult(store, v);
    EXPECT_EQ(id.valueForm(), ast::ValueForm::Scalar);
    EXPECT_FALSE(id.holdsAggregateAddress());
    EXPECT_TRUE(id.hasResultSymbol(store));
    EXPECT_EQ(store.result(&id)->getName(), "x");
}

TEST(Expression, isArrayObjectTypeFalseForScalar) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression id("x", ctx());
    id.setType(type::signedInteger());
    EXPECT_FALSE(id.isArrayObjectType());
    EXPECT_FALSE(id.hasDecayedArrayValue(store));
}

TEST(Expression, valueTypeAndGetResultAreStoreOnly) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression id("x", ctx());
    symbols::ValueEntry v("x", type::signedInteger(), false, ctx(), 0);
    id.setTypeAndResult(store, v);
    EXPECT_TRUE(id.valueType(store).isPrimitive());
    EXPECT_EQ(id.getResultSymbol(store)->getName(), "x");
    EXPECT_TRUE(id.hasResultSymbol(store));
    EXPECT_FALSE(id.hasDecayedArrayValue(store));
}

TEST(Expression, resultGoneWhenStoreCleared) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression id("x", ctx());
    symbols::ValueEntry v("x", type::signedInteger(), false, ctx(), 0);
    id.setTypeAndResult(store, v);
    store.clear();
    // Result is store-only; expression type remains on the node.
    // Probe with hasResultSymbol — getResultSymbol asserts when Result is required-missing.
    EXPECT_TRUE(id.valueType(store).isPrimitive());
    EXPECT_FALSE(id.hasResultSymbol(store));
    EXPECT_EQ(store.value(&id, symbols::ValueSlot::Result), nullptr);
    EXPECT_FALSE(id.hasDecayedArrayValue(store));
}

TEST(Expression, functionDesignatorFormWritesStore) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression id("f", ctx());
    type::Type fn = type::function(type::signedInteger());
    symbols::ValueEntry addr("t", type::pointer(fn), true, ctx(), 0);
    id.setFunctionDesignatorResult(store, addr);
    EXPECT_TRUE(id.holdsFunctionDesignator());
    EXPECT_TRUE(store.hasResult(&id));
    EXPECT_EQ(id.getResultSymbol(store)->getName(), "t");
}

} // namespace
