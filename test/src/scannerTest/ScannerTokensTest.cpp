#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "scanner/LexFileScannerReader.h"
#include "scanner/LexicalSession.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "types/Type.h"

#include "ResourceHelpers.h"

#include <string>
#include <utility>
#include <vector>

using namespace testing;
using namespace scanner;

namespace {

std::vector<Token> scanAll(const std::string &path) {
    LexFileScannerReader reader;
    LexicalSession session;
    Scanner scanner{path, reader.fromConfiguration(getResourcePath("configuration/scanner.lex")), session};
    std::vector<Token> out;
    for (int i = 0; i < 200; ++i) {
        Token t = scanner.nextToken();
        if (t.id == Token::END) {
            break;
        }
        out.push_back(t);
    }
    return out;
}

TEST(ScannerTokens, keywordsAreDistinctFromIdentifiers) {
    auto path = writeTempSource("scan_kw", "const volatile static extern typedef sizeof struct union enum "
                                           "short long signed unsigned double do switch case default goto "
                                           "inline restrict noreturn nullptr typeof "
                                           "bool true false _Complex "
                                           "int x;\n");
    auto toks = scanAll(path);
    auto has = [&](const std::string &id) {
        for (const auto &t : toks) {
            if (t.id == id && t.lexeme == id) {
                return true;
            }
        }
        return false;
    };
    EXPECT_TRUE(has("const"));
    EXPECT_TRUE(has("volatile"));
    EXPECT_TRUE(has("static"));
    EXPECT_TRUE(has("extern"));
    EXPECT_TRUE(has("typedef"));
    EXPECT_TRUE(has("sizeof"));
    EXPECT_TRUE(has("struct"));
    EXPECT_TRUE(has("union"));
    EXPECT_TRUE(has("enum"));
    EXPECT_TRUE(has("short"));
    EXPECT_TRUE(has("long"));
    EXPECT_TRUE(has("signed"));
    EXPECT_TRUE(has("unsigned"));
    EXPECT_TRUE(has("double"));
    EXPECT_TRUE(has("do"));
    EXPECT_TRUE(has("switch"));
    EXPECT_TRUE(has("case"));
    EXPECT_TRUE(has("default"));
    EXPECT_TRUE(has("goto"));
    EXPECT_TRUE(has("inline"));
    EXPECT_TRUE(has("restrict"));
    EXPECT_TRUE(has("noreturn"));
    EXPECT_TRUE(has("nullptr"));
    EXPECT_TRUE(has("typeof"));
    EXPECT_TRUE(has("bool"));
    EXPECT_TRUE(has("true"));
    EXPECT_TRUE(has("false"));
    EXPECT_TRUE(has("_Complex"));
    bool sawX = false;
    for (const auto &t : toks) {
        if (t.lexeme == "x") {
            EXPECT_EQ(t.id, "id");
            sawX = true;
        }
    }
    EXPECT_TRUE(sawX);
}

TEST(ScannerTokens, gnuBuiltinNamesAreIdentifiers) {
    auto path = writeTempSource("scan_gnu_id",
            "__builtin_va_arg __builtin_offsetof __builtin_types_compatible_p x;\n");
    auto toks = scanAll(path);
    int builtinIds = 0;
    for (const auto &t : toks) {
        if (t.lexeme.rfind("__builtin_", 0) == 0) {
            EXPECT_EQ(t.id, "id");
            ++builtinIds;
        }
    }
    EXPECT_EQ(builtinIds, 3);
}

TEST(ScannerTokens, punctuatorsDotArrowQuestionColonEllipsis) {
    auto path = writeTempSource("scan_punct", "a.b a->b c ? d : e f(...);\n");
    auto toks = scanAll(path);
    std::vector<std::string> ids;
    for (const auto &t : toks) {
        ids.push_back(t.id);
    }
    EXPECT_THAT(ids, Contains("."));
    EXPECT_THAT(ids, Contains("->"));
    EXPECT_THAT(ids, Contains("?"));
    EXPECT_THAT(ids, Contains(":"));
    EXPECT_THAT(ids, Contains("..."));
}

TEST(ScannerTokens, hexIntegerConstant) {
    auto path = writeTempSource("scan_hex", "int x = 0x2A;\n");
    auto toks = scanAll(path);
    bool found = false;
    for (const auto &t : toks) {
        if (t.id == "int_const" && (t.lexeme == "0x2A" || t.lexeme == "0x2a")) {
            found = true;
        }
    }
    EXPECT_TRUE(found) << "expected hex int_const token";
}

TEST(ScannerTokens, floatWithExponentAndSuffix) {
    auto path = writeTempSource("scan_float", "float f = 1.5e2f; double d = 1e-3;\n");
    auto toks = scanAll(path);
    bool foundFloat = false;
    bool foundExp = false;
    for (const auto &t : toks) {
        if (t.id == "float_const") {
            if (t.lexeme.find("1.5") != std::string::npos || t.lexeme.find('f') != std::string::npos || t.lexeme.find('F') != std::string::npos) {
                foundFloat = true;
            }
            if (t.lexeme.find('e') != std::string::npos || t.lexeme.find('E') != std::string::npos) {
                foundExp = true;
            }
        }
    }
    EXPECT_TRUE(foundFloat);
    EXPECT_TRUE(foundExp);
}

TEST(ScannerTokens, charHexAndOctalEscapesLexAsCharConst) {
    auto path = writeTempSource("scan_charesc", "char a = '\\x41'; char b = '\\101';\n");
    auto toks = scanAll(path);
    int charConsts = 0;
    for (const auto &t : toks) {
        if (t.id == "char_const") {
            ++charConsts;
            EXPECT_THAT(t.lexeme, AnyOf(HasSubstr("\\x41"), HasSubstr("\\101"), Eq("'\\x41'"), Eq("'\\101'")));
        }
    }
    EXPECT_GE(charConsts, 2);
}

TEST(ScannerTokens, stringWithEscapesIsOneToken) {
    auto path = writeTempSource("scan_str", "char *p = \"a\\n\\t\";\n");
    auto toks = scanAll(path);
    bool found = false;
    for (const auto &t : toks) {
        if (t.id == "string") {
            found = true;
            EXPECT_THAT(t.lexeme, HasSubstr("\\n"));
        }
    }
    EXPECT_TRUE(found);
}

TEST(ScannerTokens, stillScansExistingKeywords) {
    auto path = writeTempSource("scan_old_kw", "int main(void) { if (1) while (0) return 0; }\n");
    auto toks = scanAll(path);
    auto has = [&](const std::string &id) {
        for (const auto &t : toks) {
            if (t.id == id) {
                return true;
            }
        }
        return false;
    };
    EXPECT_TRUE(has("int"));
    EXPECT_TRUE(has("if"));
    EXPECT_TRUE(has("while"));
    EXPECT_TRUE(has("return"));
    EXPECT_TRUE(has("void"));
}


TEST(ScannerTokens, tokensCarryLineMarkerSourceAndLine) {
    auto path = writeTempSource("scan_line_marker",
            "# 10 \"orig.c\"\n"
            "int x;\n"
            "y;\n");
    auto toks = scanAll(path);
    ASSERT_GE(toks.size(), 5u);
    EXPECT_EQ(toks[0].id, "int");
    EXPECT_EQ(toks[0].lexeme, "int");
    EXPECT_EQ(toks[0].context.getSourceName(), "orig.c");
    EXPECT_EQ(toks[0].context.getOffset(), 10u);
    EXPECT_EQ(toks[1].id, "id");
    EXPECT_EQ(toks[1].lexeme, "x");
    EXPECT_EQ(toks[1].context.getSourceName(), "orig.c");
    EXPECT_EQ(toks[1].context.getOffset(), 10u);
    EXPECT_EQ(toks[2].id, ";");
    EXPECT_EQ(toks[2].context.getSourceName(), "orig.c");
    EXPECT_EQ(toks[2].context.getOffset(), 10u);
    EXPECT_EQ(toks[3].id, "id");
    EXPECT_EQ(toks[3].lexeme, "y");
    EXPECT_EQ(toks[3].context.getSourceName(), "orig.c");
    EXPECT_EQ(toks[3].context.getOffset(), 11u);
}

TEST(ScannerTokens, secondLineMarkerRetargetsFollowingTokens) {
    auto path = writeTempSource("scan_two_markers",
            "# 1 \"a.c\"\n"
            "int a;\n"
            "# 20 \"b.h\"\n"
            "char b;\n");
    auto toks = scanAll(path);
    ASSERT_GE(toks.size(), 6u);
    EXPECT_EQ(toks[0].lexeme, "int");
    EXPECT_EQ(toks[0].context.getSourceName(), "a.c");
    EXPECT_EQ(toks[0].context.getOffset(), 1u);
    EXPECT_EQ(toks[1].lexeme, "a");
    EXPECT_EQ(toks[1].context.getSourceName(), "a.c");
    EXPECT_EQ(toks[3].lexeme, "char");
    EXPECT_EQ(toks[3].context.getSourceName(), "b.h");
    EXPECT_EQ(toks[3].context.getOffset(), 20u);
    EXPECT_EQ(toks[4].lexeme, "b");
    EXPECT_EQ(toks[4].context.getSourceName(), "b.h");
    EXPECT_EQ(toks[4].context.getOffset(), 20u);
}

TEST(ScannerTokens, tokenAfterSpanningCommentUsesLineAfterComment) {
    auto path = writeTempSource("scan_span_comment",
            "int\n"
            "/* comment\n"
            "   still */\n"
            "x;\n");
    auto toks = scanAll(path);
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[0].lexeme, "int");
    EXPECT_EQ(toks[0].context.getOffset(), 1u);
    EXPECT_EQ(toks[1].lexeme, "x");
    EXPECT_EQ(toks[1].context.getOffset(), 4u);
}

TEST(ScannerTokens, longIdentifierKeepsStartLine) {
    auto path = writeTempSource("scan_long_id", "int abcdefghijklmnopqrstuvwxyz;\n");
    auto toks = scanAll(path);
    ASSERT_GE(toks.size(), 3u);
    EXPECT_EQ(toks[1].id, "id");
    EXPECT_EQ(toks[1].lexeme, "abcdefghijklmnopqrstuvwxyz");
    EXPECT_EQ(toks[1].context.getOffset(), 1u);
}

TEST(ScannerTokens, firstTokenUsesRealFilename) {
    auto path = writeTempSource("scan_real_name", "void f(void);\n");
    auto toks = scanAll(path);
    ASSERT_FALSE(toks.empty());
    EXPECT_EQ(toks[0].context.getSourceName(), path);
    EXPECT_EQ(toks[0].context.getOffset(), 1u);
}

TEST(ScannerTokens, leadingWhitespaceDoesNotShiftLine) {
    auto path = writeTempSource("scan_ws_prefix", "   int x;\n");
    auto toks = scanAll(path);
    ASSERT_GE(toks.size(), 1u);
    EXPECT_EQ(toks[0].lexeme, "int");
    EXPECT_EQ(toks[0].context.getOffset(), 1u);
}

TEST(ScannerTokens, compoundPunctuatorIsOneTokenAtStartLine) {
    auto path = writeTempSource("scan_compound", "a <<= b;\n");
    auto toks = scanAll(path);
    std::vector<std::string> ids;
    for (const auto& t : toks) {
        ids.push_back(t.id);
    }
    EXPECT_THAT(ids, ElementsAre("id", "<<=", "id", ";"));
    EXPECT_EQ(toks[1].lexeme, "<<=");
    EXPECT_EQ(toks[1].context.getOffset(), 1u);
}

TEST(ScannerTokens, emitsTypedefNameWhenSessionRegisters) {
    auto path = writeTempSource("scan_typedef_name", "myint x;\n");
    LexFileScannerReader reader;
    LexicalSession session;
    session.names.add("myint", type::signedInteger());
    Scanner scanner{path, reader.fromConfiguration(getResourcePath("configuration/scanner.lex")), session};
    Token t = scanner.nextToken();
    EXPECT_EQ(t.id, "typedef_name");
    EXPECT_EQ(t.lexeme, "myint");
}

} // namespace
