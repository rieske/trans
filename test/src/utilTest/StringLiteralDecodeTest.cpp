#include "gtest/gtest.h"

#include "util/StringLiteralDecode.h"

#include <vector>

using util::decodeCEscapes;
using util::decodeCharConstant;
using util::decodeStringLiteralBytes;
using util::encodeStringLiteralToken;
using util::stringLiteralArrayLength;
using util::toGasByteDirective;
using util::toNasmDbDirective;

TEST(StringLiteralDecode, plainStringIncludesTrailingNul) {
    auto bytes = decodeStringLiteralBytes("\"hello\"");
    ASSERT_EQ(bytes.size(), 6u);
    EXPECT_EQ(bytes[0], 'h');
    EXPECT_EQ(bytes[4], 'o');
    EXPECT_EQ(bytes[5], 0);
    EXPECT_EQ(stringLiteralArrayLength("\"hello\""), 6);
}

TEST(StringLiteralDecode, simpleEscapes) {
    auto bytes = decodeStringLiteralBytes("\"a\\n\\t\\\\\\\"\"");
    // a, \n, \t, \, ", NUL
    ASSERT_EQ(bytes.size(), 6u);
    EXPECT_EQ(bytes[0], 'a');
    EXPECT_EQ(bytes[1], '\n');
    EXPECT_EQ(bytes[2], '\t');
    EXPECT_EQ(bytes[3], '\\');
    EXPECT_EQ(bytes[4], '"');
    EXPECT_EQ(bytes[5], 0);
}

// Cover remaining single-letter escapes (Scanner string decode + util share policy).
TEST(StringLiteralDecode, additionalControlEscapes) {
    auto bytes = decodeStringLiteralBytes("\"\\r\\a\\b\\f\\v\\?\"");
    ASSERT_EQ(bytes.size(), 7u);
    EXPECT_EQ(bytes[0], '\r');
    EXPECT_EQ(bytes[1], '\a');
    EXPECT_EQ(bytes[2], '\b');
    EXPECT_EQ(bytes[3], '\f');
    EXPECT_EQ(bytes[4], '\v');
    EXPECT_EQ(bytes[5], '?');
    EXPECT_EQ(bytes[6], 0);
}

TEST(StringLiteralDecode, uppercaseHexEscape) {
    auto bytes = decodeStringLiteralBytes("\"\\x4A\\X4B\"");
    ASSERT_EQ(bytes.size(), 3u);
    EXPECT_EQ(bytes[0], 'J');
    EXPECT_EQ(bytes[1], 'K');
    EXPECT_EQ(bytes[2], 0);
}

TEST(StringLiteralDecode, hexEscape) {
    auto bytes = decodeStringLiteralBytes("\"\\x41\\x00\\xFF\"");
    // \x41='A', \x00=0, \xFF=255, plus trailing NUL from decoder
    ASSERT_EQ(bytes.size(), 4u);
    EXPECT_EQ(bytes[0], 0x41);
    EXPECT_EQ(bytes[1], 0x00);
    EXPECT_EQ(bytes[2], 0xFF);
    EXPECT_EQ(bytes[3], 0);
}

TEST(StringLiteralDecode, octalEscape) {
    auto bytes = decodeStringLiteralBytes("\"\\101\\0\"");
    // \101='A', \0=0, trailing NUL
    ASSERT_EQ(bytes.size(), 3u);
    EXPECT_EQ(bytes[0], 'A');
    EXPECT_EQ(bytes[1], 0);
    EXPECT_EQ(bytes[2], 0);
}

TEST(StringLiteralDecode, incompleteArrayLengthXPattern) {
    // char x[] = "XXXXXX" has size 7 including NUL
    EXPECT_EQ(stringLiteralArrayLength("\"XXXXXX\""), 7);
}

TEST(StringLiteralDecode, nasmDbDirective) {
    EXPECT_EQ(toNasmDbDirective("\"AB\""), "db 65, 66, 0");
    EXPECT_EQ(toNasmDbDirective("\"\\n\""), "db 10, 0");
    EXPECT_EQ(toNasmDbDirective("\"a'b\""), "db 97, 39, 98, 0");
}

TEST(StringLiteralDecode, gasByteDirective) {
    EXPECT_EQ(toGasByteDirective("\"AB\""), ".byte 65, 66, 0");
    EXPECT_EQ(toGasByteDirective("\"a'b\""), ".byte 97, 39, 98, 0");
}

TEST(StringLiteralDecode, unquotedBodyStillDecodes) {
    // Callers may pass body without quotes in some paths; treat as raw body.
    auto bytes = decodeStringLiteralBytes("hi");
    ASSERT_EQ(bytes.size(), 3u);
    EXPECT_EQ(bytes[0], 'h');
    EXPECT_EQ(bytes[1], 'i');
    EXPECT_EQ(bytes[2], 0);
}

TEST(StringLiteralDecode, encodeInteriorBytesAsQuotedToken) {
    EXPECT_EQ(encodeStringLiteralToken({'a', 'b'}), "\"ab\"");
    EXPECT_EQ(encodeStringLiteralToken({'\t', 'd'}), "\"\\td\"");
}

TEST(StringLiteralDecode, charConstantSimpleEscapes) {
    long value = 0;
    ASSERT_TRUE(util::decodeCharConstant("'\\a'", value));
    EXPECT_EQ(value, '\a');
    ASSERT_TRUE(util::decodeCharConstant("'\\f'", value));
    EXPECT_EQ(value, '\f');
    ASSERT_TRUE(util::decodeCharConstant("'\\v'", value));
    EXPECT_EQ(value, '\v');
    ASSERT_TRUE(util::decodeCharConstant("'A'", value));
    EXPECT_EQ(value, 'A');
}

TEST(StringLiteralDecode, charConstantHexAndOctal) {
    long value = 0;
    ASSERT_TRUE(util::decodeCharConstant("'\\xFE'", value));
    EXPECT_EQ(value, 0xFE);
    ASSERT_TRUE(util::decodeCharConstant("'\\033'", value));
    EXPECT_EQ(value, 27);
    ASSERT_TRUE(util::decodeCharConstant("'\\101'", value));
    EXPECT_EQ(value, 'A');
    ASSERT_TRUE(util::decodeCharConstant("'\\0'", value));
    EXPECT_EQ(value, 0);
}

TEST(StringLiteralDecode, charConstantRejectsEmptyAndMultiChar) {
    long value = 0;
    EXPECT_FALSE(util::decodeCharConstant("''", value));
    EXPECT_FALSE(util::decodeCharConstant("'ab'", value));
    EXPECT_FALSE(util::decodeCharConstant("65", value));
}

TEST(StringLiteralDecode, encodeDecodeRoundTripInterior) {
    const std::vector<unsigned char> bytes = {'a', '\t', '"', 1};
    auto decoded = decodeStringLiteralBytes(encodeStringLiteralToken(bytes));
    ASSERT_FALSE(decoded.empty());
    decoded.pop_back();
    EXPECT_EQ(decoded, bytes);
}

TEST(StringLiteralDecode, decodeCEscapesHasNoTrailingNul) {
    auto bytes = decodeCEscapes("a\\n");
    ASSERT_EQ(bytes.size(), 2u);
    EXPECT_EQ(bytes[0], 'a');
    EXPECT_EQ(bytes[1], '\n');
}

TEST(StringLiteralDecode, decodeCharConstantSimpleAndEscapes) {
    long v = -1;
    ASSERT_TRUE(decodeCharConstant("'A'", v));
    EXPECT_EQ(v, 'A');
    ASSERT_TRUE(decodeCharConstant("'\\n'", v));
    EXPECT_EQ(v, '\n');
    ASSERT_TRUE(decodeCharConstant("'\\x41'", v));
    EXPECT_EQ(v, 0x41);
    ASSERT_TRUE(decodeCharConstant("'\\033'", v));
    EXPECT_EQ(v, 033);
    EXPECT_FALSE(decodeCharConstant("'AB'", v));
    EXPECT_FALSE(decodeCharConstant("A", v));
}
