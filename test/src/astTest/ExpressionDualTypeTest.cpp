#include "gtest/gtest.h"

#include "ast/GenericSelection.h"
#include "ast/IdentifierExpression.h"
#include "ast/Operator.h"
#include "ast/UnaryExpression.h"
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
    symbols::ValueEntry addr("t", type::pointer(type::signedInteger()), ctx(), 0);
    id.setAggregateAddressResult(store, addr, arr);
    EXPECT_TRUE(id.holdsAggregateAddress());
    EXPECT_TRUE(id.isArrayObjectType());
    EXPECT_TRUE(id.hasDecayedArrayValue(store));
    EXPECT_TRUE(store.hasResult(&id));
}

TEST(Expression, setTypeAndResultIsScalar) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression id("x", ctx());
    symbols::ValueEntry v("x", type::signedInteger(), ctx(), 0);
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
    symbols::ValueEntry v("x", type::signedInteger(), ctx(), 0);
    id.setTypeAndResult(store, v);
    EXPECT_TRUE(id.valueType(store).isPrimitive());
    EXPECT_EQ(id.getResultSymbol(store)->getName(), "x");
    EXPECT_TRUE(id.hasResultSymbol(store));
    EXPECT_FALSE(id.hasDecayedArrayValue(store));
}

TEST(Expression, resultGoneWhenStoreCleared) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression id("x", ctx());
    symbols::ValueEntry v("x", type::signedInteger(), ctx(), 0);
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
    symbols::ValueEntry addr("t", type::pointer(fn), ctx(), 0);
    id.setFunctionDesignatorResult(store, addr);
    EXPECT_TRUE(id.holdsFunctionDesignator());
    EXPECT_TRUE(store.hasResult(&id));
    EXPECT_EQ(id.getResultSymbol(store)->getName(), "t");
}

TEST(Expression, takeValueFromCopiesScalarResultAndLvalue) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression src("x", ctx());
    ast::IdentifierExpression dest("g", ctx());
    symbols::ValueEntry v("x", type::signedInteger(), ctx(), 0);
    symbols::ValueEntry addr("xp", type::pointer(type::signedInteger()), ctx(), 0);
    src.setTypeAndResult(store, v);
    src.setLvalueSymbol(store, addr);
    dest.takeValueFrom(src, store);
    EXPECT_EQ(dest.valueForm(), ast::ValueForm::Scalar);
    EXPECT_TRUE(dest.expressionType().isPrimitive());
    EXPECT_EQ(dest.getResultSymbol(store)->getName(), "x");
    EXPECT_EQ(dest.getLvalueSymbol(store)->getName(), "xp");
    EXPECT_TRUE(dest.isLval());
}

TEST(Expression, takeValueFromCopiesAggregateFormAndAddressPlan) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression src("s", ctx());
    ast::IdentifierExpression dest("g", ctx());
    type::Type rec = type::structure({ { "x", type::signedInteger() } });
    symbols::ValueEntry addr("sp", type::pointer(rec), ctx(), 0);
    src.setAggregateAddressResult(store, addr, rec);
    symbols::FieldPlan field;
    field.fieldOffsetBytes = 4;
    store.setAddressPlan(&src, symbols::AddressPlan { field });
    dest.takeValueFrom(src, store);
    EXPECT_TRUE(dest.holdsAggregateAddress());
    EXPECT_TRUE(dest.expressionType().isStructure());
    EXPECT_EQ(dest.getResultSymbol(store)->getName(), "sp");
    const auto* plan = store.addressPlan(&dest);
    ASSERT_NE(plan, nullptr);
    const auto* copied = symbols::get_if<symbols::FieldPlan>(plan);
    ASSERT_NE(copied, nullptr);
    EXPECT_EQ(copied->fieldOffsetBytes, 4);
}

TEST(Expression, takeValueFromCopiesFunctionDesignatorClearsLval) {
    symbols::AnnotationStore store;
    ast::IdentifierExpression src("f", ctx());
    ast::IdentifierExpression dest("g", ctx());
    type::Type fn = type::function(type::signedInteger());
    symbols::ValueEntry addr("t", type::pointer(fn), ctx(), 0);
    src.setFunctionDesignatorResult(store, addr);
    ASSERT_TRUE(dest.isLval());
    dest.takeValueFrom(src, store);
    EXPECT_TRUE(dest.holdsFunctionDesignator());
    EXPECT_FALSE(dest.isLval());
}

TEST(UnaryExpression, dereferenceIsLvalue) {
    auto operand = std::make_unique<ast::IdentifierExpression>("p", ctx());
    ast::UnaryExpression expr(std::make_unique<ast::Operator>("*"), std::move(operand));
    EXPECT_TRUE(expr.isLval());
}

TEST(UnaryExpression, plusIsNotLvalue) {
    auto operand = std::make_unique<ast::IdentifierExpression>("x", ctx());
    ast::UnaryExpression expr(std::make_unique<ast::Operator>("+"), std::move(operand));
    EXPECT_FALSE(expr.isLval());
}

TEST(UnaryExpression, realImagLvalueIsTheFlag) {
    auto operand = std::make_unique<ast::IdentifierExpression>("z", ctx());
    ast::UnaryExpression expr(std::make_unique<ast::Operator>("__real__"), std::move(operand));
    EXPECT_FALSE(expr.isLval());
    expr.setLval(true);
    EXPECT_TRUE(expr.isLval());
}

TEST(GenericSelection, selectAdoptsValueAndCategory) {
    symbols::AnnotationStore store;
    auto controlling = std::make_unique<ast::IdentifierExpression>("c", ctx());
    auto arm = std::make_unique<ast::IdentifierExpression>("x", ctx());
    symbols::ValueEntry v("x", type::signedInteger(), ctx(), 0);
    arm->setTypeAndResult(store, v);
    ASSERT_TRUE(arm->isLval());
    std::vector<ast::GenericAssociation> associations;
    associations.push_back({ std::nullopt, std::move(arm) });
    ast::GenericSelection generic { ctx(), std::move(controlling), std::move(associations) };
    EXPECT_FALSE(generic.isLval());
    generic.select(0, store);
    EXPECT_TRUE(generic.hasSelected());
    EXPECT_TRUE(generic.isLval());
    EXPECT_EQ(generic.getResultSymbol(store)->getName(), "x");
}

} // namespace
