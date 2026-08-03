#include "gtest/gtest.h"

#include "ast/ConstantExpression.h"
#include "ast/Constant.h"
#include "ast/IdentifierExpression.h"
#include "symbols/AnnotationStore.h"
#include "symbols/ValueEntry.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

#include <stdexcept>

namespace {

translation_unit::Context ctx() { return { "test", 1 }; }

struct StoreFixture : testing::Test {
    symbols::AnnotationStore store;
};

TEST_F(StoreFixture, setTypeAndResultIsScalarForm) {
    ast::IdentifierExpression expr { "x", ctx() };
    symbols::ValueEntry v { "t1", type::signedLong(), true, ctx(), 0 };
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
            symbols::ValueEntry { "t0", arr.decayArray(), true, ctx(), 0 }, arr);
    EXPECT_EQ(expr.valueForm(), ast::ValueForm::AggregateAddress);
    EXPECT_TRUE(expr.holdsAggregateAddress());
    EXPECT_TRUE(expr.isArrayObjectType());
    EXPECT_TRUE(expr.valueType(store).isPointer());
    // hasDecayedArrayValue is derived from form + types for tests; CG uses holdsAggregateAddress.
    EXPECT_TRUE(expr.hasDecayedArrayValue(store));
}

TEST_F(StoreFixture, functionDesignatorFormStoresNameOnPlan) {
    ast::IdentifierExpression expr { "f", ctx() };
    auto ptr = type::pointer(type::function(type::signedInteger(), {}));
    expr.setFunctionDesignatorResult(store, symbols::ValueEntry { "t0", ptr, true, ctx(), 0 }, "foo");
    EXPECT_TRUE(expr.holdsFunctionDesignator());
    EXPECT_EQ(expr.valueForm(), ast::ValueForm::FunctionDesignator);
    ASSERT_NE(expr.functionDesignatorName(store), nullptr);
    EXPECT_EQ(*expr.functionDesignatorName(store), "foo");
    EXPECT_FALSE(expr.holdsAggregateAddress());
}

TEST_F(StoreFixture, plainArrayIdentifierIsScalarNotAggregateAddress) {
    ast::IdentifierExpression expr { "a", ctx() };
    auto arr = type::array(type::signedInteger(), 4);
    expr.setTypeAndResult(store, symbols::ValueEntry { "a", arr, false, ctx(), 0 });
    EXPECT_EQ(expr.valueForm(), ast::ValueForm::Scalar);
    EXPECT_FALSE(expr.holdsAggregateAddress());
    EXPECT_FALSE(expr.hasDecayedArrayValue(store));
}

TEST_F(StoreFixture, valueTypeRequiresResultSlot) {
    ast::ConstantExpression expr { ast::Constant { "1", type::signedInteger(), ctx() } };
    expr.setType(type::signedLong());
    EXPECT_FALSE(expr.hasResult(store));
    // Post-SA value type is Result-only; no soft fallback to expressionType.
    EXPECT_THROW(expr.valueType(store), std::runtime_error);
    expr.setTypeAndResult(store, symbols::ValueEntry { "t", type::signedLong(), true, ctx(), 0 });
    EXPECT_TRUE(expr.valueType(store).equivalentTo(type::signedLong()));
}

TEST_F(StoreFixture, decayAndConversionUseTypedStringSlots) {
    ast::IdentifierExpression expr { "a", ctx() };
    store.setString(&expr, symbols::StringSlot::ArrayDecaySource, "buf");
    store.setString(&expr, symbols::StringSlot::ConversionTarget, "conv");
    EXPECT_EQ(*store.string(&expr, symbols::StringSlot::ArrayDecaySource), "buf");
    EXPECT_EQ(*store.string(&expr, symbols::StringSlot::ConversionTarget), "conv");
}

} // namespace
