#include "gtest/gtest.h"

#include "util/ImmediateFormat.h"
#include "util/FloatingLiteral.h"

TEST(ImmediateFormat, wordImmediateUsesHexAboveSigned32) {
    EXPECT_EQ(util::wordImmediate(42), "42");
    EXPECT_EQ(util::wordImmediate(0x7fffffffull), "2147483647");
    EXPECT_EQ(util::wordImmediate(0x80000000ull), "0x80000000");
}

TEST(FloatingLiteral, immediateIsDoubleBitsHex) {
    std::string imm;
    ASSERT_TRUE(util::floatingLiteralImmediate("1.0", imm));
    EXPECT_EQ(imm, "0x3ff0000000000000");
    ASSERT_TRUE(util::floatingLiteralImmediate("2.5e1", imm));
    EXPECT_EQ(imm, util::hexImmediate(0x4039000000000000ull));
    EXPECT_FALSE(util::floatingLiteralImmediate("not-a-float", imm));
}

TEST(FloatingLiteral, floatSuffixIs32Bit) {
    std::string imm;
    ASSERT_TRUE(util::floatingLiteralImmediate("1.0f", imm));
    EXPECT_EQ(imm, "0x3f800000");
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("2.5f", parsed));
    EXPECT_EQ(parsed.sizeBytes, 4);
    EXPECT_EQ(parsed.bits, 0x40200000ull);
}

TEST(FloatingLiteral, negativeHasSignBit) {
    util::FloatingBits parsed;
    ASSERT_TRUE(util::floatingLiteralBits("-1.5", parsed));
    EXPECT_EQ(parsed.sizeBytes, 8);
    EXPECT_EQ(parsed.bits >> 63, 1ull);
    EXPECT_EQ(util::hexImmediate(parsed.bits), "0xbff8000000000000");
}
