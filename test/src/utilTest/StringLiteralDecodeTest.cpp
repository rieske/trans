#include "gtest/gtest.h"

#include "util/StringLiteralDecode.h"

using util::decodeStringLiteralBytes;
using util::stringLiteralArrayLength;
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
}

TEST(StringLiteralDecode, unquotedBodyStillDecodes) {
    // Callers may pass body without quotes in some paths; treat as raw body.
    auto bytes = decodeStringLiteralBytes("hi");
    ASSERT_EQ(bytes.size(), 3u);
    EXPECT_EQ(bytes[0], 'h');
    EXPECT_EQ(bytes[1], 'i');
    EXPECT_EQ(bytes[2], 0);
}
