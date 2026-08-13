#include "gtest/gtest.h"

#include "util/ImmediateFormat.h"
#include "util/FloatingLiteral.h"

#include <cstring>

TEST(ImmediateFormat, wordImmediateUsesHexAboveSigned32) {
    EXPECT_EQ(util::wordImmediate(42), "42");
    EXPECT_EQ(util::wordImmediate(0x7fffffffull), "2147483647");
    EXPECT_EQ(util::wordImmediate(0x80000000ull), "0x80000000");
}

TEST(ImmediateFormat, integerLiteralImmediateStripsCSuffixes) {
    std::string imm;
    ASSERT_TRUE(util::integerLiteralImmediate("0xff00000000000000ULL", imm));
    EXPECT_EQ(imm, "0xff00000000000000");
    ASSERT_TRUE(util::integerLiteralImmediate("18446744073709551615UL", imm));
    EXPECT_EQ(imm, "0xffffffffffffffff");
    ASSERT_TRUE(util::integerLiteralImmediate("42u", imm));
    EXPECT_EQ(imm, "42");
    EXPECT_FALSE(util::integerLiteralImmediate("not-an-int", imm));
}

TEST(FloatingLiteral, immediateIsDoubleBitsHex) {
    std::string imm;
    ASSERT_TRUE(util::floatingLiteralImmediate("1.0", imm));
    EXPECT_EQ(imm, "0x3ff0000000000000");
    ASSERT_TRUE(util::floatingLiteralImmediate("2.5e1", imm));
    EXPECT_EQ(imm, util::hexImmediate(0x4039000000000000ull));
    EXPECT_FALSE(util::floatingLiteralImmediate("not-a-float", imm));
}

TEST(FloatingLiteral, longDoubleSuffixMatchesHostEncoding) {
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("6.0L", parsed));
    EXPECT_EQ(parsed.sizeBytes, 16);
    long double expect = 6.0L;
    unsigned long long words[2] = { 0, 0 };
    std::memcpy(words, &expect, 10);
    EXPECT_EQ(parsed.bits, words[0]);
    EXPECT_EQ(parsed.bitsHi, words[1]);
}

TEST(FloatingLiteral, floatSuffixIs32Bit) {
    std::string imm;
    ASSERT_TRUE(util::floatingLiteralImmediate("1.0f", imm));
    EXPECT_EQ(imm, "0x3f800000");
    EXPECT_FALSE(util::floatingLiteralImmediate("1.0L", imm));
    EXPECT_FALSE(util::floatingLiteralImmediate("42.5l", imm));
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("2.5f", parsed));
    EXPECT_EQ(parsed.sizeBytes, 4);
    EXPECT_EQ(parsed.bits, 0x40200000ull);
}

TEST(FloatingLiteral, encodeDecodeRoundTrip) {
    const util::FloatingBits f32 = util::encodeFloating(2.0, 4);
    EXPECT_EQ(f32.sizeBytes, 4);
    EXPECT_EQ(f32.bits, 0x40000000ull);
    EXPECT_EQ(util::decodeFloating(f32), 2.0);
    const util::FloatingBits f64 = util::encodeFloating(2.5, 8);
    EXPECT_EQ(f64.sizeBytes, 8);
    EXPECT_EQ(util::decodeFloating(f64), 2.5);
}

TEST(FloatingLiteral, encodeDecodeLongDouble) {
    const util::FloatingBits ld = util::encodeFloating(6.0, 16);
    EXPECT_EQ(ld.sizeBytes, 16);
    EXPECT_EQ(util::decodeFloating(ld), 6.0);
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("6.0L", parsed));
    EXPECT_EQ(parsed.bits, ld.bits);
    EXPECT_EQ(parsed.bitsHi, ld.bitsHi);
}

TEST(FloatingLiteral, twoWordImmediateRejected) {
    std::string imm;
    EXPECT_FALSE(util::floatingLiteralImmediate("6.0L", imm));
}

TEST(FloatingLiteral, imaginarySuffixStripsAndKeepsWidth) {
    EXPECT_TRUE(util::hasImaginarySuffix("2.0i"));
    EXPECT_TRUE(util::hasImaginarySuffix("1.0I"));
    EXPECT_TRUE(util::hasImaginarySuffix("1.0if"));
    EXPECT_FALSE(util::hasImaginarySuffix("1.0"));
    EXPECT_EQ(util::stripFloatSuffix("2.0i"), "2.0");
    EXPECT_EQ(util::stripFloatSuffix("1.0if"), "1.0");
    EXPECT_EQ(util::floatingLiteralSizeBytes("1.0if"), 4);
    EXPECT_EQ(util::floatingLiteralSizeBytes("2.0i"), 8);
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("2.0i", parsed));
    EXPECT_EQ(parsed.sizeBytes, 8);
    EXPECT_EQ(parsed.bits, 0x4000000000000000ull);
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

TEST(FloatingLiteral, negativeHasSignBit) {
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("-1.5", parsed));
    EXPECT_EQ(parsed.sizeBytes, 8);
    EXPECT_EQ(parsed.bits >> 63, 1ull);
    EXPECT_EQ(util::hexImmediate(parsed.bits), "0xbff8000000000000");
}

TEST(FloatingLiteral, convertPreservesIntPastDoubleMantissa) {
    const util::FloatingBits ld = util::encodeFloating(9007199254740993.0L, 16);
    EXPECT_EQ(util::decodeFloating(ld), 9007199254740993.0L);
    const util::FloatingBits back = util::convertFloating(ld, 16);
    EXPECT_EQ(back.bits, ld.bits);
    EXPECT_EQ(back.bitsHi, ld.bitsHi);
}

TEST(FloatingLiteral, negateFlipsSignBitWithoutNarrowing) {
    const util::FloatingBits pos = util::encodeFloating(6.0, 16);
    const util::FloatingBits neg = util::negateFloating(pos);
    EXPECT_EQ(neg.sizeBytes, 16);
    EXPECT_EQ(neg.bits, pos.bits);
    EXPECT_EQ(neg.bitsHi, pos.bitsHi ^ 0x8000ull);
    EXPECT_EQ(util::negateFloating(neg).bitsHi, pos.bitsHi);
    const util::FloatingBits f32 = util::negateFloating(util::encodeFloating(1.0, 4));
    EXPECT_EQ(f32.bits, 0xbf800000ull);
}
