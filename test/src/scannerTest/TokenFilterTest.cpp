#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/LexFileScannerReader.h"
#include "scanner/LexicalSession.h"
#include "scanner/Scanner.h"
#include "scanner/TokenFilter.h"
#include "scanner/Token.h"

#include "ResourceHelpers.h"

#include <string>
#include <utility>
#include <vector>

using namespace testing;
using namespace scanner;

namespace {

std::vector<std::pair<std::string, std::string>> filterIds(const std::string& path) {
    LexFileScannerReader reader;
    LexicalSession session;
    Scanner scanner { path,
            reader.fromConfiguration(getResourcePath("configuration/scanner.lex")), session };
    TokenFilter filter { [&scanner]() { return scanner.nextToken(); } };
    std::vector<std::pair<std::string, std::string>> out;
    for (;;) {
        Token t = filter.nextToken();
        if (t.id == Token::END) {
            break;
        }
        out.push_back({ t.id, t.lexeme });
    }
    return out;
}

TEST(TokenFilter, stripsAttributeAndKeepsDeclaration) {
    auto path = writeTempSource("tf_attr", "int x __attribute__((unused));\n");
    auto toks = filterIds(path);
    ASSERT_THAT(toks.size(), Ge(3u));
    EXPECT_EQ(toks[0].second, "int");
    EXPECT_EQ(toks[1].second, "x");
    EXPECT_EQ(toks[2].second, ";");
    for (const auto& t : toks) {
        EXPECT_THAT(t.second, Not(HasSubstr("__attribute__")));
        EXPECT_NE(t.second, "unused");
    }
}

TEST(TokenFilter, preservesAttributeTextInsideStringLiteral) {
    auto path = writeTempSource("tf_attr_str",
            "const char *p = \"__attribute__((unused))\";\n");
    auto toks = filterIds(path);
    bool foundString = false;
    for (const auto& t : toks) {
        if (t.first == "string") {
            foundString = true;
            EXPECT_THAT(t.second, HasSubstr("__attribute__"));
        }
        EXPECT_NE(t.second, "__attribute__");
    }
    EXPECT_TRUE(foundString);
}

TEST(TokenFilter, dropsExtensionMarker) {
    auto path = writeTempSource("tf_ext", "__extension__ int f(void) { return 1; }\n");
    auto toks = filterIds(path);
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__extension__");
    }
    ASSERT_FALSE(toks.empty());
    EXPECT_EQ(toks[0].second, "int");
}

TEST(TokenFilter, mapsGnuConstToConstKeyword) {
    auto path = writeTempSource("tf_const", "__const int k = 3;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].first, "const");
    EXPECT_EQ(toks[0].second, "const");
    EXPECT_EQ(toks[1].second, "int");
}

TEST(TokenFilter, mapsGnuInlineToInlineKeyword) {
    auto path = writeTempSource("tf_ginline", "__inline__ int f(void);\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].first, "inline");
    EXPECT_EQ(toks[0].second, "inline");
    EXPECT_EQ(toks[1].second, "int");
}

TEST(TokenFilter, mapsGnuRestrictToRestrictKeyword) {
    auto path = writeTempSource("tf_grestrict", "int * __restrict__ p;\n");
    auto toks = filterIds(path);
    bool sawRestrict = false;
    for (const auto& t : toks) {
        if (t.first == "restrict") {
            sawRestrict = true;
        }
        EXPECT_NE(t.second, "__restrict__");
    }
    EXPECT_TRUE(sawRestrict);
}

TEST(TokenFilter, stripsAsmBalancedForm) {
    auto path = writeTempSource("tf_asm", "int y __asm__(\"x\");\n");
    auto toks = filterIds(path);
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__asm__");
    }
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].second, "int");
    EXPECT_EQ(toks[1].second, "y");
    EXPECT_EQ(toks[2].second, ";");
}

TEST(TokenFilter, stripsWideStringPrefixL) {
    auto path = writeTempSource("tf_wide_l", "const char *p = L\"NULL\";\n");
    auto toks = filterIds(path);
    bool found = false;
    for (const auto& t : toks) {
        if (t.first == "string") {
            found = true;
            EXPECT_EQ(t.second, "\"NULL\"");
        }
        EXPECT_NE(t.second, "L");
    }
    EXPECT_TRUE(found);
}

TEST(TokenFilter, concatenatesAdjacentStringLiterals) {
    auto path = writeTempSource("tf_concat", "const char *p = \"ab\" \"cd\";\n");
    auto toks = filterIds(path);
    int stringCount = 0;
    for (const auto& t : toks) {
        if (t.first == "string") {
            ++stringCount;
            EXPECT_EQ(t.second, "\"abcd\"");
        }
    }
    EXPECT_EQ(stringCount, 1);
}

TEST(TokenFilter, singletonHexStringKeepsSourceSpelling) {
    auto path = writeTempSource("tf_hex_one", "const char *p = \"\\x09\";\n");
    auto toks = filterIds(path);
    int stringCount = 0;
    for (const auto& t : toks) {
        if (t.first == "string") {
            ++stringCount;
            EXPECT_EQ(t.second, "\"\\x09\"");
        }
    }
    EXPECT_EQ(stringCount, 1);
}

TEST(TokenFilter, asmWithoutParenGroupRestoresPrefixes) {
    auto path = writeTempSource("tf_asm_noparen", "asm volatile x;\n");
    auto toks = filterIds(path);
    bool sawAsm = false;
    bool sawVolatile = false;
    bool sawX = false;
    for (const auto& t : toks) {
        if (t.second == "asm" || t.second == "__asm__") {
            sawAsm = true;
        }
        if (t.first == "volatile") {
            sawVolatile = true;
        }
        if (t.second == "x") {
            sawX = true;
        }
    }
    EXPECT_FALSE(sawAsm);
    EXPECT_TRUE(sawVolatile);
    EXPECT_TRUE(sawX);
}

TEST(TokenFilter, stripsAsmWithGnuVolatilePrefix) {
    auto path = writeTempSource("tf_asm_gvol", "int y __asm__ __volatile__(\"x\");\n");
    auto toks = filterIds(path);
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__asm__");
        EXPECT_NE(t.second, "__volatile__");
        EXPECT_NE(t.first, "volatile");
    }
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].second, "int");
    EXPECT_EQ(toks[1].second, "y");
    EXPECT_EQ(toks[2].second, ";");
}

TEST(TokenFilter, stripsAsmWithExtensionThenVolatile) {
    auto path = writeTempSource("tf_asm_extvol", "asm __extension__ volatile (\"nop\");\n");
    auto toks = filterIds(path);
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "asm");
        EXPECT_NE(t.second, "__extension__");
        EXPECT_NE(t.first, "volatile");
        EXPECT_NE(t.second, "nop");
    }
    ASSERT_EQ(toks.size(), 1u);
    EXPECT_EQ(toks[0].second, ";");
}

TEST(TokenFilter, hexStringConcatDoesNotMergeEscapes) {
    auto path = writeTempSource("tf_hex_concat",
            "const char *p = \"\\x09\" \"def\";\n");
    auto toks = filterIds(path);
    int stringCount = 0;
    for (const auto& t : toks) {
        if (t.first == "string") {
            ++stringCount;
            EXPECT_EQ(t.second, "\"\\tdef\"");
        }
    }
    EXPECT_EQ(stringCount, 1);
}

TEST(TokenFilter, concatenatesAcrossExtensionMarker) {
    auto path = writeTempSource("tf_concat_ext",
            "const char *p = \"ab\" __extension__ \"cd\";\n");
    auto toks = filterIds(path);
    int stringCount = 0;
    for (const auto& t : toks) {
        if (t.first == "string") {
            ++stringCount;
            EXPECT_EQ(t.second, "\"abcd\"");
        }
        EXPECT_NE(t.second, "__extension__");
    }
    EXPECT_EQ(stringCount, 1);
}

TEST(TokenFilter, stripsAsmBareSpelling) {
    auto path = writeTempSource("tf_asm_bare", "int y __asm(\"x\");\n");
    auto toks = filterIds(path);
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__asm");
        EXPECT_NE(t.second, "asm");
    }
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].second, "int");
    EXPECT_EQ(toks[1].second, "y");
    EXPECT_EQ(toks[2].second, ";");
}

TEST(TokenFilter, mapsGnuSignedToSignedKeyword) {
    auto path = writeTempSource("tf_signed", "__signed__ int k;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].first, "signed");
    EXPECT_EQ(toks[0].second, "signed");
    EXPECT_EQ(toks[1].second, "int");
}

TEST(TokenFilter, mapsInt128ToLongKeyword) {
    auto path = writeTempSource("tf_i128", "__int128 x;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].first, "long");
    EXPECT_EQ(toks[0].second, "long");
    EXPECT_EQ(toks[1].second, "x");
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__int128");
    }
}

TEST(TokenFilter, mapsUnsignedInt128ToUnsignedLong) {
    auto path = writeTempSource("tf_u128", "unsigned __int128 x;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].second, "unsigned");
    EXPECT_EQ(toks[1].first, "long");
    EXPECT_EQ(toks[1].second, "long");
    EXPECT_EQ(toks[2].second, "x");
}

TEST(TokenFilter, mapsUint128tToUnsignedLong) {
    auto path = writeTempSource("tf_u128t", "__uint128_t x;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].first, "unsigned");
    EXPECT_EQ(toks[1].first, "long");
    EXPECT_EQ(toks[2].second, "x");
}

} // namespace
