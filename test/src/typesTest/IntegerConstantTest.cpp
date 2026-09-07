#include "gtest/gtest.h"

#include "types/IntegerConstant.h"
#include "types/Operator.h"

namespace {

TEST(IntegerConstant, convertBoolAndNarrowSigned) {
    EXPECT_EQ(type::toHostLong(type::convert(type::fromHostLong(2), type::boolean())), 1);
    EXPECT_EQ(type::toHostLong(type::convert(type::fromHostLong(0), type::boolean())), 0);
    EXPECT_EQ(type::toHostLong(type::convert(type::fromHostLong(2), type::signedInteger())), 2);
}

TEST(IntegerConstant, convertTruncatesToDestWidth) {
    EXPECT_EQ(type::toHostLong(type::convert(type::fromHostLong(-1), type::unsignedInteger())),
            0xffffffffL);
    EXPECT_EQ(type::toHostLong(type::convert(
                      type::fromHostLong(static_cast<long>(0x8000000000000000ULL)),
                      type::unsignedInteger())),
            0L);
    EXPECT_EQ(type::toHostLong(type::convert(type::fromHostLong(-1), type::unsignedLong())), -1L);
    EXPECT_EQ(type::toHostLong(type::convert(type::fromHostLong(-1), type::signedInteger())), -1L);
    EXPECT_EQ(type::toHostLong(type::convert(type::fromHostLong(-1), type::signedCharacter())),
            -1L);
}

TEST(IntegerConstant, fromLiteralBitsDoesNotInventSourceType) {
    auto uns = type::fromLiteralBits(0xffffffff, type::unsignedInteger());
    EXPECT_TRUE(uns.type.equivalentTo(type::unsignedInteger()));
    EXPECT_EQ(type::toHostLong(uns), 0xffffffffL);
}

TEST(IntegerConstant, convertLeavesNonIntegralSourceAlone) {
    type::IntegerConstant src { 7, type::signedInteger() };
    auto out = type::convert(src, type::floating());
    EXPECT_TRUE(out.type.equivalentTo(type::signedInteger()));
    EXPECT_EQ(type::toHostLong(out), 7);
}

TEST(IntegerConstant, foldUnaryLogicalNotAndArithmetic) {
    auto two = type::fromHostLong(2);
    auto zero = type::fromHostLong(0);
    auto foldedNot = type::foldUnary(type::UnaryOp::LogicalNot, two);
    ASSERT_TRUE(foldedNot.has_value());
    EXPECT_EQ(type::toHostLong(*foldedNot), 0);
    EXPECT_TRUE(foldedNot->type.equivalentTo(type::signedInteger()));

    auto foldedZero = type::foldUnary(type::UnaryOp::LogicalNot, zero);
    ASSERT_TRUE(foldedZero.has_value());
    EXPECT_EQ(type::toHostLong(*foldedZero), 1);

    auto plus = type::foldUnary(type::UnaryOp::Plus, two);
    ASSERT_TRUE(plus.has_value());
    EXPECT_EQ(type::toHostLong(*plus), 2);

    auto minus = type::foldUnary(type::UnaryOp::Minus, two);
    ASSERT_TRUE(minus.has_value());
    EXPECT_EQ(type::toHostLong(*minus), -2);

    auto bitNot = type::foldUnary(type::UnaryOp::BitNot, zero);
    ASSERT_TRUE(bitNot.has_value());
    EXPECT_EQ(type::toHostLong(*bitNot), -1);
}

TEST(IntegerConstant, foldUnaryRejectsUnknownAndNonIntegral) {
    EXPECT_FALSE(type::foldUnary(type::UnaryOp::Deref, type::fromHostLong(1)).has_value());
    EXPECT_FALSE(type::foldUnary(type::UnaryOp::Addr, type::fromHostLong(1)).has_value());
    EXPECT_FALSE(type::foldUnary(type::UnaryOp::Sizeof, type::fromHostLong(1)).has_value());
    type::IntegerConstant fp { 0, type::floating() };
    EXPECT_FALSE(type::foldUnary(type::UnaryOp::Minus, fp).has_value());
    auto notFp = type::foldUnary(type::UnaryOp::LogicalNot, fp);
    ASSERT_TRUE(notFp.has_value());
    EXPECT_EQ(type::toHostLong(*notFp), 1);
}

TEST(IntegerConstant, foldBinaryArithmeticBitwiseAndCompare) {
    auto left = type::fromHostLong(7);
    auto right = type::fromHostLong(3);
    auto add = type::foldBinary(type::BinaryOp::Add, left, right);
    ASSERT_TRUE(add.has_value());
    EXPECT_EQ(type::toHostLong(*add), 10);

    auto sub = type::foldBinary(type::BinaryOp::Sub, left, right);
    ASSERT_TRUE(sub.has_value());
    EXPECT_EQ(type::toHostLong(*sub), 4);

    auto mul = type::foldBinary(type::BinaryOp::Mul, left, right);
    ASSERT_TRUE(mul.has_value());
    EXPECT_EQ(type::toHostLong(*mul), 21);

    auto div = type::foldBinary(type::BinaryOp::Div, left, right);
    ASSERT_TRUE(div.has_value());
    EXPECT_EQ(type::toHostLong(*div), 2);

    auto mod = type::foldBinary(type::BinaryOp::Mod, left, right);
    ASSERT_TRUE(mod.has_value());
    EXPECT_EQ(type::toHostLong(*mod), 1);

    auto band = type::foldBinary(type::BinaryOp::BitAnd, left, right);
    ASSERT_TRUE(band.has_value());
    EXPECT_EQ(type::toHostLong(*band), 3);

    auto bor = type::foldBinary(type::BinaryOp::BitOr, left, right);
    ASSERT_TRUE(bor.has_value());
    EXPECT_EQ(type::toHostLong(*bor), 7);

    auto bxor = type::foldBinary(type::BinaryOp::BitXor, left, right);
    ASSERT_TRUE(bxor.has_value());
    EXPECT_EQ(type::toHostLong(*bxor), 4);

    auto lt = type::foldBinary(type::BinaryOp::Lt, left, right);
    ASSERT_TRUE(lt.has_value());
    EXPECT_EQ(type::toHostLong(*lt), 0);
    EXPECT_TRUE(lt->type.equivalentTo(type::signedInteger()));

    auto ge = type::foldBinary(type::BinaryOp::Ge, left, right);
    ASSERT_TRUE(ge.has_value());
    EXPECT_EQ(type::toHostLong(*ge), 1);

    auto eq = type::foldBinary(type::BinaryOp::Eq, left, left);
    ASSERT_TRUE(eq.has_value());
    EXPECT_EQ(type::toHostLong(*eq), 1);

    auto ne = type::foldBinary(type::BinaryOp::Ne, left, right);
    ASSERT_TRUE(ne.has_value());
    EXPECT_EQ(type::toHostLong(*ne), 1);
}

TEST(IntegerConstant, foldBinaryShiftLogicalAndRejects) {
    auto one = type::fromHostLong(1);
    auto two = type::fromHostLong(2);
    auto shl = type::foldBinary(type::BinaryOp::Shl, one, two);
    ASSERT_TRUE(shl.has_value());
    EXPECT_EQ(type::toHostLong(*shl), 4);

    auto shr = type::foldBinary(type::BinaryOp::Shr, type::fromHostLong(8), two);
    ASSERT_TRUE(shr.has_value());
    EXPECT_EQ(type::toHostLong(*shr), 2);

    auto land = type::foldBinary(type::BinaryOp::LogAnd, one, two);
    ASSERT_TRUE(land.has_value());
    EXPECT_EQ(type::toHostLong(*land), 1);

    auto lor = type::foldBinary(type::BinaryOp::LogOr, type::fromHostLong(0), two);
    ASSERT_TRUE(lor.has_value());
    EXPECT_EQ(type::toHostLong(*lor), 1);

    EXPECT_FALSE(type::foldBinary(type::BinaryOp::Div, one, type::fromHostLong(0)).has_value());
    EXPECT_FALSE(type::foldBinary(type::BinaryOp::Mod, one, type::fromHostLong(0)).has_value());
    EXPECT_FALSE(type::foldBinary(type::BinaryOp::Shl, one, type::fromHostLong(-1)).has_value());
}

TEST(IntegerConstant, enumUnderlyingTypeSelectsByRange) {
    auto v = [](long n) {
        return type::signedValue(type::fromHostLong(n));
    };
    EXPECT_TRUE(type::enumUnderlyingType(v(0), v(1)).equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::enumUnderlyingType(v(-1), v(1)).equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::enumUnderlyingType(v(0), v(0x80000000L)).equivalentTo(type::unsignedInteger()));
    EXPECT_TRUE(type::enumUnderlyingType(v(0x100000000L), v(0x100000000L)).equivalentTo(type::signedLong()));
    EXPECT_TRUE(type::enumUnderlyingType(v(0), v(0x100000000L)).equivalentTo(type::signedLong()));
    EXPECT_TRUE(type::enumUnderlyingType(v(42), v(42)).equivalentTo(type::signedInteger()));
    EXPECT_TRUE(type::enumUnderlyingType(v(0x80000000L), v(0x80000000L)).equivalentTo(
            type::unsignedInteger()));
}

} // namespace
