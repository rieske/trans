#include "gtest/gtest.h"

#include "symbols/StaticInit.h"

#include <variant>

namespace {

TEST(StaticInit, asDataWordsSplitsWideFloat) {
    const symbols::StaticFloat fp { 0x8000000000000000ull, 0x3fffull, 16 };
    const auto words = symbols::asDataWords(fp);
    ASSERT_EQ(words.size(), 2u);
    const auto* lo = std::get_if<symbols::StaticWord>(&words[0]);
    const auto* hi = std::get_if<symbols::StaticWord>(&words[1]);
    ASSERT_NE(lo, nullptr);
    ASSERT_NE(hi, nullptr);
    EXPECT_EQ(lo->bits, 0x8000000000000000ull);
    EXPECT_EQ(hi->bits, 0x3fffull);
}

TEST(StaticInit, asDataWordsNarrowFloatIsOneWord) {
    const symbols::StaticFloat fp { 0x3f800000ull, 0, 4 };
    const auto words = symbols::asDataWords(fp);
    ASSERT_EQ(words.size(), 1u);
    const auto* word = std::get_if<symbols::StaticWord>(&words[0]);
    ASSERT_NE(word, nullptr);
    EXPECT_EQ(word->bits, 0x3f800000ull);
}

TEST(StaticInit, asDataWordsDoubleIsOneWord) {
    const symbols::StaticFloat fp { 0x3ff0000000000000ull, 0, 8 };
    const auto words = symbols::asDataWords(fp);
    ASSERT_EQ(words.size(), 1u);
    const auto* word = std::get_if<symbols::StaticWord>(&words[0]);
    ASSERT_NE(word, nullptr);
    EXPECT_EQ(word->bits, 0x3ff0000000000000ull);
}

TEST(StaticInit, asDataWordsIntegerIsOneWord) {
    const auto words = symbols::asDataWords(symbols::StaticInteger { -3 });
    ASSERT_EQ(words.size(), 1u);
    const auto* word = std::get_if<symbols::StaticWord>(&words[0]);
    ASSERT_NE(word, nullptr);
    EXPECT_EQ(word->bits, static_cast<unsigned long long>(-3));
}

TEST(StaticInit, asDataWordsSplitsWideInteger) {
    type::IntegerConstant ice = type::fromLiteralBits(type::Bits(1) << 64, type::signedInt128());
    const auto words = symbols::asDataWords(symbols::StaticInteger { ice });
    ASSERT_EQ(words.size(), 2u);
    const auto* lo = std::get_if<symbols::StaticWord>(&words[0]);
    const auto* hi = std::get_if<symbols::StaticWord>(&words[1]);
    ASSERT_NE(lo, nullptr);
    ASSERT_NE(hi, nullptr);
    EXPECT_EQ(lo->bits, 0ull);
    EXPECT_EQ(hi->bits, 1ull);
}

TEST(StaticInit, asDataWordsAddressUnchanged) {
    const auto words = symbols::asDataWords(symbols::StaticAddress { "foo", 4 });
    ASSERT_EQ(words.size(), 1u);
    const auto* addr = std::get_if<symbols::StaticAddress>(&words[0]);
    ASSERT_NE(addr, nullptr);
    EXPECT_EQ(addr->symbol, "foo");
    EXPECT_EQ(addr->addend, 4);
}

} // namespace
