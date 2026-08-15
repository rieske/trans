#include "gtest/gtest.h"

#include "types/IntegerConstant.h"

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
