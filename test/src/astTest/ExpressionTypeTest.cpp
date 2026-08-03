#include "gtest/gtest.h"

#include "ast/AssignmentExpression.h"
#include "ast/ConstantExpression.h"
#include "ast/Constant.h"
#include "ast/GenericSelection.h"
#include "ast/IdentifierExpression.h"
#include "ast/Operator.h"
#include "symbols/AddressPlan.h"
#include "symbols/AnnotationStore.h"
#include "symbols/ValueEntry.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

#include <memory>
#include <stdexcept>
#include <vector>

namespace {

translation_unit::Context ctx() { return { "test", 1 }; }

struct StoreFixture : testing::Test {
    symbols::AnnotationStore store;
};

TEST_F(StoreFixture, setTypeAndResultIsScalarForm) {
    ast::IdentifierExpression expr { "x", ctx() };
    symbols::ValueEntry v { "t1", type::signedLong(), ctx(), 0 };
    expr.setTypeAndResult(store, v);
    EXPECT_EQ(expr.valueForm(), ast::ValueForm::Scalar);
    EXPECT_FALSE(expr.holdsAggregateAddress());
    EXPECT_FALSE(expr.holdsFunctionDesignator());
    // Dual-type probe matches form for scalars with array type only when pointer result.
    EXPECT_FALSE(expr.hasDecayedArrayValue(store));
}

TEST_F(StoreFixture, aggregateAddressFormIsCanonicalDualTypeSignal) {
    ast::IdentifierExpression expr { "row", ctx() };
    auto arr = type::array(type::signedInteger(), 3);
    expr.setAggregateAddressResult(store,
            symbols::ValueEntry { "t0", arr.decayArray(), ctx(), 0 }, arr);
    EXPECT_EQ(expr.valueForm(), ast::ValueForm::AggregateAddress);
    EXPECT_TRUE(expr.holdsAggregateAddress());
    EXPECT_TRUE(expr.isArrayObjectType());
    EXPECT_TRUE(expr.valueType(store).isPointer());
    // hasDecayedArrayValue is derived from form + types for tests; CG uses holdsAggregateAddress.
    EXPECT_TRUE(expr.hasDecayedArrayValue(store));
}

TEST_F(StoreFixture, functionDesignatorFormStoresNameOnPlan) {
    ast::IdentifierExpression expr { "f", ctx() };
    type::Type fn = type::function(type::signedInteger(), {});
    auto ptr = type::pointer(fn);
    expr.setFunctionDesignatorResult(store, symbols::ValueEntry { "t0", ptr, ctx(), 0 }, fn);
    symbols::FunctionDesignatorPlan plan;
    plan.functionName = "foo";
    store.setAddressPlan(&expr, symbols::AddressPlan { plan });
    EXPECT_TRUE(expr.holdsFunctionDesignator());
    EXPECT_EQ(expr.valueForm(), ast::ValueForm::FunctionDesignator);
    const auto* stored = symbols::get_if<symbols::FunctionDesignatorPlan>(store.addressPlan(&expr));
    ASSERT_NE(stored, nullptr);
    ASSERT_TRUE(stored->functionName.has_value());
    EXPECT_EQ(*stored->functionName, "foo");
    EXPECT_FALSE(expr.holdsAggregateAddress());
}

TEST_F(StoreFixture, plainArrayIdentifierIsScalarNotAggregateAddress) {
    ast::IdentifierExpression expr { "a", ctx() };
    auto arr = type::array(type::signedInteger(), 4);
    expr.setTypeAndResult(store, symbols::ValueEntry { "a", arr, ctx(), 0 });
    EXPECT_EQ(expr.valueForm(), ast::ValueForm::Scalar);
    EXPECT_FALSE(expr.holdsAggregateAddress());
    EXPECT_FALSE(expr.hasDecayedArrayValue(store));
}

TEST_F(StoreFixture, valueTypeRequiresResultSlot) {
    ast::ConstantExpression expr { ast::Constant { "1", type::signedInteger(), ctx() } };
    expr.setType(type::signedLong());
    EXPECT_FALSE(expr.hasResultSymbol(store));
    EXPECT_TRUE(expr.valueType(store).equivalentTo(type::signedLong()));
    expr.setTypeAndResult(store, symbols::ValueEntry { "t", type::signedLong(), ctx(), 0 });
    EXPECT_TRUE(expr.valueType(store).equivalentTo(type::signedLong()));
}

TEST_F(StoreFixture, constantLabelAndTypedConversionSlot) {
    ast::IdentifierExpression expr { "a", ctx() };
    store.setString(&expr, symbols::StringSlot::ConstantLabel, "buf");
    store.setValue(&expr, symbols::ValueSlot::Conversion,
            symbols::ValueEntry { "conv", type::boolean(), ctx(), 0 });
    EXPECT_EQ(*store.string(&expr, symbols::StringSlot::ConstantLabel), "buf");
    ASSERT_NE(store.value(&expr, symbols::ValueSlot::Conversion), nullptr);
    EXPECT_EQ(store.value(&expr, symbols::ValueSlot::Conversion)->getName(), "conv");
    EXPECT_TRUE(store.value(&expr, symbols::ValueSlot::Conversion)->getType().equivalentTo(
            type::boolean()));
}



TEST_F(StoreFixture, takeValueFromCopiesScalarResultAndLvalue) {
    ast::IdentifierExpression src { "x", ctx() };
    ast::IdentifierExpression dest { "g", ctx() };
    src.setTypeAndResult(store, symbols::ValueEntry { "x", type::signedInteger(), ctx(), 0 });
    store.setValue(&src, symbols::ValueSlot::Lvalue,
            symbols::ValueEntry { "xp", type::pointer(type::signedInteger()), ctx(), 0 });
    dest.takeValueFrom(src, store);
    EXPECT_EQ(dest.valueForm(), ast::ValueForm::Scalar);
    EXPECT_TRUE(dest.expressionType().isPrimitive());
    ASSERT_TRUE(dest.hasResultSymbol(store));
    EXPECT_EQ(dest.getResultSymbol(store)->getName(), "x");
    ASSERT_NE(dest.getLvalueSymbol(store), nullptr);
    EXPECT_EQ(dest.getLvalueSymbol(store)->getName(), "xp");
    EXPECT_TRUE(dest.isLval());
}

TEST_F(StoreFixture, takeValueFromCopiesAggregateFormAndAddressPlan) {
    ast::IdentifierExpression src { "s", ctx() };
    ast::IdentifierExpression dest { "g", ctx() };
    type::Type rec = type::structure({ { "x", type::signedInteger() } });
    src.setAggregateAddressResult(store,
            symbols::ValueEntry { "sp", type::pointer(rec), ctx(), 0 }, rec);
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

TEST_F(StoreFixture, takeValueFromCopiesFunctionDesignatorClearsLval) {
    ast::IdentifierExpression src { "f", ctx() };
    ast::IdentifierExpression dest { "g", ctx() };
    type::Type fn = type::function(type::signedInteger());
    src.setFunctionDesignatorResult(store,
            symbols::ValueEntry { "t", type::pointer(fn), ctx(), 0 }, fn);
    ASSERT_TRUE(dest.isLval());
    dest.takeValueFrom(src, store);
    EXPECT_TRUE(dest.holdsFunctionDesignator());
    EXPECT_FALSE(dest.isLval());
}

TEST_F(StoreFixture, genericSelectionSelectAdoptsValueAndCategory) {
    auto controlling = std::make_unique<ast::IdentifierExpression>("c", ctx());
    auto arm = std::make_unique<ast::IdentifierExpression>("x", ctx());
    arm->setTypeAndResult(store, symbols::ValueEntry { "x", type::signedInteger(), ctx(), 0 });
    ASSERT_TRUE(arm->isLval());
    std::vector<ast::GenericAssociation> associations;
    associations.push_back({ std::nullopt, std::move(arm) });
    ast::GenericSelection generic { ctx(), std::move(controlling), std::move(associations) };
    EXPECT_FALSE(generic.isLval());
    generic.select(0, store);
    EXPECT_TRUE(generic.hasSelected());
    EXPECT_TRUE(generic.isLval());
    ASSERT_TRUE(generic.hasResultSymbol(store));
    EXPECT_EQ(generic.getResultSymbol(store)->getName(), "x");
}

TEST_F(StoreFixture, assignmentExpressionIsNeverLvalue) {
    auto left = std::make_unique<ast::IdentifierExpression>("x", ctx());
    auto right = std::make_unique<ast::IdentifierExpression>("y", ctx());
    ast::AssignmentExpression assign { std::move(left), std::make_unique<ast::Operator>("="), std::move(right) };
    EXPECT_FALSE(assign.isLval());
    EXPECT_TRUE(assign.getLeftOperand()->isLval());
}

} // namespace
