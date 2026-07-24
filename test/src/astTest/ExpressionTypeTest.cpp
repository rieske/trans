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
    symbols::AnnotationStore::Bind bind { store };
};

TEST_F(StoreFixture, setTypedResultSetsBothTypeAndValue) {
    ast::IdentifierExpression expr { "x", ctx() };
    symbols::ValueEntry v { "t1", type::signedLong(), true, ctx(), 0 };
    expr.setTypedResult(v);
    EXPECT_TRUE(expr.expressionType().equivalentTo(type::signedLong()));
    EXPECT_TRUE(expr.valueType().equivalentTo(type::signedLong()));
}

TEST_F(StoreFixture, setResultSymbolRequiresPriorSetType) {
    ast::IdentifierExpression expr { "x", ctx() };
    symbols::ValueEntry v { "t1", type::signedInteger(), true, ctx(), 0 };
    EXPECT_THROW(expr.setResultSymbol(v), std::runtime_error);
}

TEST_F(StoreFixture, dualOwnershipPreservesExpressionType) {
    ast::IdentifierExpression expr { "x", ctx() };
    auto arr = type::array(type::signedCharacter(), 4);
    auto ptr = arr.decayArray();
    symbols::ValueEntry ptrVal { "t0", ptr, true, ctx(), 0 };

    expr.setType(arr);
    expr.setResultSymbol(ptrVal);
    EXPECT_TRUE(expr.expressionType().isArray());
    EXPECT_TRUE(expr.valueType().isPointer());
}

TEST_F(StoreFixture, valueTypeFallsBackToExpressionTypeWithoutResult) {
    ast::ConstantExpression expr { ast::Constant { "1", type::signedInteger(), ctx() } };
    expr.setType(type::signedLong());
    EXPECT_FALSE(expr.hasResultSymbol());
    EXPECT_TRUE(expr.valueType().equivalentTo(type::signedLong()));
}

} // namespace
