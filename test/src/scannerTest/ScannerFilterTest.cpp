#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/LexFileScannerReader.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "TokenMatcher.h"

#include "ResourceHelpers.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace testing;
using namespace scanner;

namespace {

// test/programs/tmp/ is gitignored; ensureTestProgramsTmpDir creates it on a
// clean CI checkout (same helper as SourceProgram).
std::string writeTempSource(const std::string& name, const std::string& contents) {
    ensureTestProgramsTmpDir();
    std::string path = getTestProgramsTmpDir() + name + ".src";
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Could not write temp source " + path);
    }
    out << contents;
    return path;
}

std::vector<std::pair<std::string, std::string>> scanIds(const std::string& path) {
    LexFileScannerReader reader;
    Scanner scanner {
        path,
        reader.fromConfiguration(getResourcePath("configuration/scanner.lex"))
    };
    std::vector<std::pair<std::string, std::string>> out;
    for (int i = 0; i < 200; ++i) {
        Token t = scanner.nextToken();
        if (t.id == Token::END) {
            break;
        }
        out.push_back({ t.id, t.lexeme });
    }
    return out;
}

TEST(ScannerFilter, stripsAttributeAndKeepsDeclaration) {
    auto path = writeTempSource("scan_attr",
            "int x __attribute__((unused));\n");
    auto toks = scanIds(path);
    ASSERT_THAT(toks.size(), Ge(3u));
    EXPECT_EQ(toks[0].second, "int");
    EXPECT_EQ(toks[1].second, "x");
    EXPECT_EQ(toks[2].second, ";");
    for (const auto& t : toks) {
        EXPECT_THAT(t.second, Not(HasSubstr("__attribute__")));
        EXPECT_NE(t.second, "unused");
    }
}

TEST(ScannerFilter, preservesAttributeTextInsideStringLiteral) {
    auto path = writeTempSource("scan_attr_str",
            "const char *p = \"__attribute__((unused))\";\n");
    auto toks = scanIds(path);
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

TEST(ScannerFilter, dropsExtensionAndInlineMarkers) {
    auto path = writeTempSource("scan_ext",
            "__extension__ __inline__ int f(void) { return 1; }\n");
    auto toks = scanIds(path);
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__extension__");
        EXPECT_NE(t.second, "__inline__");
        EXPECT_NE(t.second, "inline");
    }
    ASSERT_FALSE(toks.empty());
    EXPECT_EQ(toks[0].second, "int");
}

TEST(ScannerFilter, mapsGnuConstToConstKeyword) {
    auto path = writeTempSource("scan_const",
            "__const int k = 3;\n");
    auto toks = scanIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].first, "const");
    EXPECT_EQ(toks[0].second, "const");
    EXPECT_EQ(toks[1].second, "int");
}

TEST(ScannerFilter, dropsPragmaLines) {
    auto path = writeTempSource("scan_pragma",
            "#pragma once\nint x;\n");
    auto toks = scanIds(path);
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "pragma");
        EXPECT_NE(t.second, "once");
    }
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].second, "int");
    EXPECT_EQ(toks[1].second, "x");
}

TEST(ScannerFilter, stripsAsmBalancedForm) {
    auto path = writeTempSource("scan_asm",
            "int y __asm__(\"x\");\n");
    auto toks = scanIds(path);
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__asm__");
    }
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].second, "int");
    EXPECT_EQ(toks[1].second, "y");
    EXPECT_EQ(toks[2].second, ";");
}

TEST(ScannerFilter, dropsC99InlineKeyword) {
    auto path = writeTempSource("scan_inline",
            "static inline int f(void) { return 1; }\n");
    auto toks = scanIds(path);
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "inline");
    }
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].second, "static");
    EXPECT_EQ(toks[1].second, "int");
}

TEST(ScannerFilter, preservesInlineInsideStringLiteral) {
    auto path = writeTempSource("scan_inline_str",
            "const char *p = \"inline \";\n");
    auto toks = scanIds(path);
    bool found = false;
    for (const auto& t : toks) {
        if (t.first == "string") {
            found = true;
            EXPECT_THAT(t.second, HasSubstr("inline"));
        }
    }
    EXPECT_TRUE(found);
}

// --- Phase B: lexical features formerly implemented as source string rewrites ---

TEST(ScannerFilter, stripsWideStringPrefixL) {
    auto path = writeTempSource("scan_wide_l",
            "const char *p = L\"NULL\";\n");
    auto toks = scanIds(path);
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

TEST(ScannerFilter, stripsU8StringPrefix) {
    auto path = writeTempSource("scan_wide_u8",
            "const char *p = u8\"hi\";\n");
    auto toks = scanIds(path);
    bool found = false;
    for (const auto& t : toks) {
        if (t.first == "string") {
            found = true;
            EXPECT_EQ(t.second, "\"hi\"");
        }
        EXPECT_NE(t.second, "u8");
    }
    EXPECT_TRUE(found);
}

TEST(ScannerFilter, concatenatesAdjacentStringLiterals) {
    auto path = writeTempSource("scan_concat",
            "const char *p = \"ab\" \"cd\";\n");
    auto toks = scanIds(path);
    int stringCount = 0;
    for (const auto& t : toks) {
        if (t.first == "string") {
            ++stringCount;
            EXPECT_EQ(t.second, "\"abcd\"");
        }
    }
    EXPECT_EQ(stringCount, 1);
}

TEST(ScannerFilter, hexStringConcatDoesNotMergeEscapes) {
    // C phases: convert escapes then concatenate. "\\x09" "def" must not become
    // one hex escape \\x09def.
    auto path = writeTempSource("scan_hex_concat",
            "const char *p = \"\\x09\" \"def\";\n");
    auto toks = scanIds(path);
    int stringCount = 0;
    for (const auto& t : toks) {
        if (t.first == "string") {
            ++stringCount;
            EXPECT_THAT(t.second, Not(HasSubstr("\\x09def")));
            EXPECT_THAT(t.second, HasSubstr("def"));
        }
    }
    EXPECT_EQ(stringCount, 1);
}

TEST(ScannerFilter, concatenatesWideAndPlainAdjacentStrings) {
    auto path = writeTempSource("scan_wide_concat",
            "const char *p = L\"hi\" \"there\";\n");
    auto toks = scanIds(path);
    int stringCount = 0;
    for (const auto& t : toks) {
        if (t.first == "string") {
            ++stringCount;
            EXPECT_EQ(t.second, "\"hithere\"");
        }
        EXPECT_NE(t.second, "L");
    }
    EXPECT_EQ(stringCount, 1);
}

TEST(ScannerFilter, mapsBoolToUnsignedCharTokens) {
    auto path = writeTempSource("scan_bool", "_Bool b;\n");
    auto toks = scanIds(path);
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].second, "unsigned");
    EXPECT_EQ(toks[1].second, "char");
    EXPECT_EQ(toks[2].second, "b");
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "_Bool");
    }
}

// linux/types.h after gcc -E: typedef __signed__ __int128 __s128 ...
// Approximate as long long so the typedef name is not eaten as a type.
TEST(ScannerFilter, mapsInt128ToLongLongTokens) {
    auto path = writeTempSource("scan_int128",
            "typedef __signed__ __int128 __s128;\n"
            "typedef unsigned __int128 __u128;\n");
    auto toks = scanIds(path);
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__int128");
    }
    // signed long long __s128 ; unsigned long long __u128 ;
    ASSERT_GE(toks.size(), 10u);
    EXPECT_EQ(toks[0].second, "typedef");
    EXPECT_EQ(toks[1].second, "signed");
    EXPECT_EQ(toks[2].second, "long");
    EXPECT_EQ(toks[3].second, "long");
    EXPECT_EQ(toks[4].second, "__s128");
    EXPECT_EQ(toks[5].second, ";");
    EXPECT_EQ(toks[6].second, "typedef");
    EXPECT_EQ(toks[7].second, "unsigned");
    EXPECT_EQ(toks[8].second, "long");
    EXPECT_EQ(toks[9].second, "long");
    EXPECT_EQ(toks[10].second, "__u128");
}

// glibc floatN after gcc -E: map ISO/IEC TS 18661 spellings to float/double.
TEST(ScannerFilter, mapsExtendedFloatTypes) {
    auto path = writeTempSource("scan_floatn",
            "_Float32 a; _Float64 b; _Float128 c; _Float32x d; _Float64x e;\n");
    auto toks = scanIds(path);
    for (const auto& t : toks) {
        EXPECT_THAT(t.second, Not(HasSubstr("_Float")));
    }
    ASSERT_GE(toks.size(), 10u);
    EXPECT_EQ(toks[0].second, "float");
    EXPECT_EQ(toks[1].second, "a");
    EXPECT_EQ(toks[3].second, "double");
    EXPECT_EQ(toks[4].second, "b");
    EXPECT_EQ(toks[6].second, "double");
    EXPECT_EQ(toks[7].second, "c");
    EXPECT_EQ(toks[9].second, "double");
    EXPECT_EQ(toks[10].second, "d");
    EXPECT_EQ(toks[12].second, "double");
    EXPECT_EQ(toks[13].second, "e");
}

TEST(ScannerFilter, doesNotRewriteXmlBoolSuffix) {
    auto path = writeTempSource("scan_xml_bool", "XML_Bool flag;\n");
    auto toks = scanIds(path);
    ASSERT_GE(toks.size(), 2u);
    EXPECT_EQ(toks[0].second, "XML_Bool");
    EXPECT_EQ(toks[1].second, "flag");
    for (const auto& t : toks) {
        EXPECT_THAT(t.second, Not(HasSubstr("XMLunsigned")));
    }
}

TEST(ScannerFilter, mapsFuncIdentifiersToEmptyString) {
    auto path = writeTempSource("scan_func",
            "const char *a = __func__;\n"
            "const char *b = __FUNCTION__;\n"
            "const char *c = __PRETTY_FUNCTION__;\n");
    auto toks = scanIds(path);
    int emptyStrings = 0;
    for (const auto& t : toks) {
        EXPECT_NE(t.second, "__func__");
        EXPECT_NE(t.second, "__FUNCTION__");
        EXPECT_NE(t.second, "__PRETTY_FUNCTION__");
        if (t.first == "string" && t.second == "\"\"") {
            ++emptyStrings;
        }
    }
    EXPECT_EQ(emptyStrings, 3);
}

TEST(ScannerFilter, preservesFuncTextInsideStringLiteral) {
    auto path = writeTempSource("scan_func_str",
            "const char *msg = \"call __func__ here\";\n");
    auto toks = scanIds(path);
    bool found = false;
    for (const auto& t : toks) {
        if (t.first == "string") {
            found = true;
            EXPECT_THAT(t.second, HasSubstr("__func__"));
        }
    }
    EXPECT_TRUE(found);
}

} // namespace
