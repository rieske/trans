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

std::vector<std::pair<std::string, std::string>> filterIds(const std::string& path, bool gnuExtensions = true) {
    LexFileScannerReader reader;
    LexicalSession session;
    Scanner scanner { path,
            reader.fromConfiguration(getResourcePath("configuration/scanner.lex")), session };
    TokenFilter filter { [&scanner]() { return scanner.nextToken(); }, gnuExtensions };
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

TEST(TokenFilter, complexIsItsOwnKeyword) {
    auto path = writeTempSource("tf_complex", "_Complex float z;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].first, "_Complex");
    EXPECT_EQ(toks[0].second, "_Complex");
    EXPECT_EQ(toks[1].second, "float");
}

TEST(TokenFilter, mapsC99BoolToBoolKeyword) {
    auto path = writeTempSource("tf_bool", "_Bool flag;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].first, "bool");
    EXPECT_EQ(toks[0].second, "bool");
    EXPECT_EQ(toks[1].second, "flag");
}

TEST(TokenFilter, mapsC11NoreturnToNoreturnKeyword) {
    auto path = writeTempSource("tf_noreturn", "_Noreturn void die(void);\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].first, "noreturn");
    EXPECT_EQ(toks[0].second, "noreturn");
    EXPECT_EQ(toks[1].second, "void");
}

TEST(TokenFilter, mapsGnuRestrictToRestrictKeyword) {
    auto path = writeTempSource("tf_grestrict", "int * __restrict__ p;\n");
    auto toks = filterIds(path);
    bool sawRestrict = false;
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__restrict__");
        EXPECT_NE(t.second, "__restrict");
        if (t.first == "restrict" && t.second == "restrict") {
            sawRestrict = true;
        }
    }
    EXPECT_TRUE(sawRestrict);
}

TEST(TokenFilter, float64StandInIsDoubleKeyword) {
    auto path = writeTempSource("tf_f64", "_Float64 y;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].second, "double");
    EXPECT_EQ(toks[1].second, "y");
}

TEST(TokenFilter, xmlBoolIdIsNotRewritten) {
    auto path = writeTempSource("tf_xmlbool", "XML_Bool ok;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].second, "XML_Bool");
    EXPECT_EQ(toks[1].second, "ok");
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

TEST(TokenFilter, leavesInt128AsIdentifier) {
    auto path = writeTempSource("tf_i128", "__int128 x;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].first, "id");
    EXPECT_EQ(toks[0].second, "__int128");
    EXPECT_EQ(toks[1].second, "x");
}

TEST(TokenFilter, leavesUnsignedInt128AsIdentifier) {
    auto path = writeTempSource("tf_u128", "unsigned __int128 x;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].second, "unsigned");
    EXPECT_EQ(toks[1].first, "id");
    EXPECT_EQ(toks[1].second, "__int128");
    EXPECT_EQ(toks[2].second, "x");
}

TEST(TokenFilter, mapsGnuTypeofToTypeofKeyword) {
    auto path = writeTempSource("tf_typeof", "__typeof__(int) x;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 6u);
    EXPECT_EQ(toks[0].first, "typeof");
    EXPECT_EQ(toks[0].second, "typeof");
    EXPECT_EQ(toks[1].second, "(");
    EXPECT_EQ(toks[2].second, "int");
    EXPECT_EQ(toks[3].second, ")");
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__typeof__");
    }
}

TEST(TokenFilter, leavesBuiltinTypesCompatiblePAsIdentifier) {
    auto path = writeTempSource("tf_types_compat",
            "__builtin_types_compatible_p(int, int);\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 1u);
    EXPECT_EQ(toks[0].first, "id");
    EXPECT_EQ(toks[0].second, "__builtin_types_compatible_p");
}

TEST(TokenFilter, mapsGnuTypeofBareToTypeofKeyword) {
    auto path = writeTempSource("tf_typeof2", "__typeof (int) x;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 1u);
    EXPECT_EQ(toks[0].first, "typeof");
    EXPECT_EQ(toks[0].second, "typeof");
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__typeof");
    }
}

TEST(TokenFilter, leavesUint128tAsIdentifier) {
    auto path = writeTempSource("tf_u128t", "__uint128_t x;\n");
    auto toks = filterIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].first, "id");
    EXPECT_EQ(toks[0].second, "__uint128_t");
    EXPECT_EQ(toks[1].second, "x");
}

TEST(TokenFilter, isoModeKeepsGnuTokens) {
    auto path = writeTempSource("tf_iso_gnu", "__extension__ __inline int x __attribute__((unused));\n");
    auto toks = filterIds(path, false);
    bool sawExtension = false;
    bool sawInline = false;
    bool sawAttribute = false;
    for (const auto& t : toks) {
        if (t.second == "__extension__") {
            sawExtension = true;
        }
        if (t.second == "__inline") {
            sawInline = true;
        }
        if (t.second == "__attribute__") {
            sawAttribute = true;
        }
    }
    EXPECT_TRUE(sawExtension);
    EXPECT_TRUE(sawInline);
    EXPECT_TRUE(sawAttribute);
}

TEST(TokenFilter, isoModeStillMapsBoolAndNoreturn) {
    auto path = writeTempSource("tf_iso_kw", "_Bool b; _Noreturn void f(void);\n");
    auto toks = filterIds(path, false);
    bool sawBool = false;
    bool sawNoreturn = false;
    for (const auto& t : toks) {
        if (t.first == "bool") {
            sawBool = true;
        }
        if (t.first == "noreturn") {
            sawNoreturn = true;
        }
        EXPECT_NE(t.second, "_Bool");
        EXPECT_NE(t.second, "_Noreturn");
    }
    EXPECT_TRUE(sawBool);
    EXPECT_TRUE(sawNoreturn);
}

} // namespace
