#include "gtest/gtest.h"

#include <memory>

#include "ast/ArithmeticExpression.h"
#include "ast/BitwiseExpression.h"
#include "ast/ComparisonExpression.h"
#include "ast/ConstantExpression.h"
#include "ast/LogicalAndExpression.h"
#include "ast/LogicalOrExpression.h"
#include "ast/Operator.h"
#include "ast/ShiftExpression.h"
#include "ast/UnaryExpression.h"
#include "types/Type.h"

namespace {

using namespace ast;

translation_unit::Context ctx() {
    return translation_unit::Context { "test", 1 };
}

std::unique_ptr<Expression> iconst(long v) {
    if (v >= 0) {
        return std::make_unique<ConstantExpression>(
                Constant { std::to_string(v), type::signedInteger(), ctx() });
    }
    auto pos = std::make_unique<ConstantExpression>(
            Constant { std::to_string(-v), type::signedInteger(), ctx() });
    return std::make_unique<UnaryExpression>(
            std::make_unique<Operator>("-"), std::move(pos));
}

// evaluateConstant must dispatch on OperatorKind (not lexeme string compares).
TEST(DoubleOperandConstantFold, arithmeticViaOperatorKind) {
    long value = 0;
    auto expr = std::make_unique<ArithmeticExpression>(
            iconst(10), std::make_unique<Operator>("+"), iconst(3));
    ASSERT_TRUE(expr->foldToHostLong(value));
    EXPECT_EQ(value, 13);

    expr = std::make_unique<ArithmeticExpression>(
            iconst(10), std::make_unique<Operator>("-"), iconst(3));
    ASSERT_TRUE(expr->foldToHostLong(value));
    EXPECT_EQ(value, 7);

    expr = std::make_unique<ArithmeticExpression>(
            iconst(10), std::make_unique<Operator>("*"), iconst(3));
    ASSERT_TRUE(expr->foldToHostLong(value));
    EXPECT_EQ(value, 30);

    expr = std::make_unique<ArithmeticExpression>(
            iconst(10), std::make_unique<Operator>("/"), iconst(3));
    ASSERT_TRUE(expr->foldToHostLong(value));
    EXPECT_EQ(value, 3);

    expr = std::make_unique<ArithmeticExpression>(
            iconst(10), std::make_unique<Operator>("%"), iconst(3));
    ASSERT_TRUE(expr->foldToHostLong(value));
    EXPECT_EQ(value, 1);
}

TEST(DoubleOperandConstantFold, shiftsBitwiseCompareLogicalViaOperatorKind) {
    long value = 0;
    auto shl = std::make_unique<ShiftExpression>(
            iconst(1), std::make_unique<Operator>("<<"), iconst(3));
    ASSERT_TRUE(shl->foldToHostLong(value));
    EXPECT_EQ(value, 8);

    auto band = std::make_unique<BitwiseExpression>(
            iconst(0xf), std::make_unique<Operator>("&"), iconst(0x3));
    ASSERT_TRUE(band->foldToHostLong(value));
    EXPECT_EQ(value, 3);

    auto cmp = std::make_unique<ComparisonExpression>(
            iconst(2), std::make_unique<Operator>("<"), iconst(5));
    ASSERT_TRUE(cmp->foldToHostLong(value));
    EXPECT_EQ(value, 1);

    auto land = std::make_unique<LogicalAndExpression>(iconst(1), iconst(0));
    ASSERT_TRUE(land->foldToHostLong(value));
    EXPECT_EQ(value, 0);

    auto lor = std::make_unique<LogicalOrExpression>(iconst(0), iconst(2));
    ASSERT_TRUE(lor->foldToHostLong(value));
    EXPECT_EQ(value, 1);
}

TEST(DoubleOperandConstantFold, divByZeroFails) {
    long value = 99;
    auto expr = std::make_unique<ArithmeticExpression>(
            iconst(1), std::make_unique<Operator>("/"), iconst(0));
    EXPECT_FALSE(expr->foldToHostLong(value));
}

std::unique_ptr<Expression> uconst(long v, type::Type ty) {
    const auto bits = static_cast<unsigned long>(v);
    return std::make_unique<ConstantExpression>(
            Constant { std::to_string(bits), std::move(ty), ctx() });
}

TEST(ConstantExpression, tokenIsCIntegerLexemeNotHostDecimal) {
    ConstantExpression c { Constant { "-1", type::signedInteger(), ctx() } };
    long value = 0;
    EXPECT_FALSE(c.foldToHostLong(value));
}

TEST(ComparisonExpression, unsignedMinusOneGreaterThanZero) {
    long value = 0;
    auto cmp = std::make_unique<ComparisonExpression>(
            uconst(-1, type::unsignedLong()), std::make_unique<Operator>(">"), iconst(0));
    ASSERT_TRUE(cmp->foldToHostLong(value));
    EXPECT_EQ(value, 1);
}

TEST(ComparisonExpression, signedMinusOneGreaterThanZeroIsFalse) {
    long value = 1;
    auto cmp = std::make_unique<ComparisonExpression>(
            iconst(-1), std::make_unique<Operator>(">"), iconst(0));
    ASSERT_TRUE(cmp->foldToHostLong(value));
    EXPECT_EQ(value, 0);
}

} // namespace
