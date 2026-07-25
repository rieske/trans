#include "gtest/gtest.h"

#include "ast/IdentifierExpression.h"
#include "semantic_analyzer/ValueEntry.h"
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
    ast::IdentifierExpression id("x", ctx());
    id.setType(type::signedInteger());
    EXPECT_TRUE(id.valueType().isPrimitive());
    EXPECT_FALSE(id.hasResultSymbol());
}

TEST(Expression, hasDecayedArrayValue) {
    ast::IdentifierExpression id("a", ctx());
    type::Type arr = type::array(type::signedInteger(), 3);
    semantic_analyzer::ValueEntry addr("t", type::pointer(type::signedInteger()), true, ctx(), 0);
    id.setAggregateAddressResult(addr, arr);
    EXPECT_TRUE(id.holdsAggregateAddress());
    EXPECT_TRUE(id.isArrayObjectType());
    EXPECT_TRUE(id.hasDecayedArrayValue());
}

TEST(Expression, setTypeAndResultIsScalar) {
    ast::IdentifierExpression id("x", ctx());
    semantic_analyzer::ValueEntry v("x", type::signedInteger(), false, ctx(), 0);
    id.setTypeAndResult(v);
    EXPECT_EQ(id.valueForm(), ast::ValueForm::Scalar);
    EXPECT_FALSE(id.holdsAggregateAddress());
    EXPECT_TRUE(id.hasResultSymbol());
}

TEST(Expression, isArrayObjectTypeFalseForScalar) {
    ast::IdentifierExpression id("x", ctx());
    id.setType(type::signedInteger());
    EXPECT_FALSE(id.isArrayObjectType());
    EXPECT_FALSE(id.hasDecayedArrayValue());
}

} // namespace
