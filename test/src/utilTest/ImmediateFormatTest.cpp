#include "gtest/gtest.h"

#include "util/ImmediateFormat.h"
#include "util/FloatingLiteral.h"
#include "util/IntegerLiteral.h"

TEST(ImmediateFormat, wordImmediateUsesHexAboveSigned32) {
    EXPECT_EQ(util::wordImmediate(42), "42");
    EXPECT_EQ(util::wordImmediate(0x7fffffffull), "2147483647");
    EXPECT_EQ(util::wordImmediate(0x80000000ull), "0x80000000");
}

TEST(FloatingLiteral, floatSuffixIs32Bit) {
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("2.5f", parsed));
    EXPECT_EQ(parsed.sizeBytes, 4);
    EXPECT_EQ(parsed.bits, 0x40200000ull);
}

TEST(FloatingLiteral, rejectsANonFloatingLexeme) {
    util::FloatingBits parsed;
    EXPECT_FALSE(util::floatingLiteralBits("not-a-float", parsed));
}

TEST(IntegerLiteral, stripsCSuffixes) {
    util::IntegerLiteral lit;
    ASSERT_TRUE(util::parseIntegerLiteral("42u", lit));
    EXPECT_EQ(static_cast<unsigned long long>(lit.value), 42ull);
    ASSERT_TRUE(util::parseIntegerLiteral("18446744073709551615UL", lit));
    EXPECT_EQ(static_cast<unsigned long long>(lit.value), 0xffffffffffffffffull);
}

TEST(IntegerLiteral, recordsTheBaseOfAHexOrOctalLexeme) {
    util::IntegerLiteral lit;
    ASSERT_TRUE(util::parseIntegerLiteral("0xff00000000000000ULL", lit));
    EXPECT_EQ(lit.base, 16);
    EXPECT_EQ(static_cast<unsigned long long>(lit.value), 0xff00000000000000ull);
    ASSERT_TRUE(util::parseIntegerLiteral("010", lit));
    EXPECT_EQ(lit.base, 8);
    EXPECT_EQ(static_cast<unsigned long long>(lit.value), 8ull);
    ASSERT_TRUE(util::parseIntegerLiteral("42", lit));
    EXPECT_EQ(lit.base, 10);
}

TEST(IntegerLiteral, rejectsANonIntegerLexeme) {
    util::IntegerLiteral lit;
    EXPECT_FALSE(util::parseIntegerLiteral("not-an-int", lit));
}

TEST(FloatingLiteral, encodeDecodeRoundTrip) {
    EXPECT_EQ(util::decodeFloating(util::encodeFloating(2.0, 4), 4), 2.0);
    EXPECT_EQ(util::decodeFloating(util::encodeFloating(2.5, 8), 8), 2.5);
    EXPECT_EQ(util::encodeFloating(2.0, 4), 0x40000000ull);
}

TEST(FloatingLiteral, oneBitsFromSize) {
    util::FloatingBits one4 = util::floatingOne(4);
    EXPECT_EQ(one4.sizeBytes, 4);
    EXPECT_EQ(one4.bits, 0x3f800000ull);
    EXPECT_EQ(one4.bitsHi, 0ull);

    util::FloatingBits one8 = util::floatingOne(8);
    EXPECT_EQ(one8.sizeBytes, 8);
    EXPECT_EQ(one8.bits, 0x3ff0000000000000ull);
    EXPECT_EQ(one8.bitsHi, 0ull);

    util::FloatingBits one16 = util::floatingOne(16);
    EXPECT_EQ(one16.sizeBytes, 16);
    EXPECT_EQ(one16.bits, 0x8000000000000000ull);
    EXPECT_EQ(one16.bitsHi, 0x3fffull);
}

TEST(FloatingLiteral, longDoubleSuffixIs80Bit) {
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("1.0L", parsed));
    EXPECT_EQ(parsed.sizeBytes, 16);
    EXPECT_EQ(parsed.bits, 0x8000000000000000ull);
    EXPECT_EQ(parsed.bitsHi, 0x3fffull);
    ASSERT_TRUE(util::floatingLiteralBits("42.5l", parsed));
    EXPECT_EQ(parsed.sizeBytes, 16);
    EXPECT_EQ(parsed.bits, 0xaa00000000000000ull);
    EXPECT_EQ(parsed.bitsHi, 0x4004ull);
}

TEST(FloatingLiteral, decodeLongDoubleRoundTrip) {
    util::FloatingBits one = util::floatingOne(16);
    EXPECT_EQ(util::decodeFloatingBits(one), 1.0);
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("1.0L", parsed));
    EXPECT_EQ(util::decodeFloatingBits(parsed), 1.0);
    ASSERT_TRUE(util::floatingLiteralBits("-3.0L", parsed));
    EXPECT_EQ(util::decodeFloatingBits(parsed), -3.0);
}

TEST(FloatingLiteral, negativeHasSignBit) {
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("-1.5", parsed));
    EXPECT_EQ(parsed.sizeBytes, 8);
    EXPECT_EQ(parsed.bits >> 63, 1ull);
    EXPECT_EQ(util::hexImmediate(parsed.bits), "0xbff8000000000000");
}
