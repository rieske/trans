#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "scanner/LexFileScannerReader.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"

#include "ResourceHelpers.h"

#include <cerrno>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <utility>
#include <vector>

using namespace testing;
using namespace scanner;

namespace {

std::string writeTempSource(const std::string &name, const std::string &contents) {
    // test/programs/tmp is gitignored; create if needed (same path layout as SourceProgram).
    const std::string dir = getTestResourcePath("programs/tmp/");
    if (mkdir(dir.c_str(), 0777) == -1 && errno != 17) {
        throw std::runtime_error("Could not create " + dir);
    }
    const std::string path = dir + name + ".src";
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Could not write " + path);
    }
    out << contents;
    return path;
}

std::vector<Token> scanAll(const std::string &path) {
    LexFileScannerReader reader;
    Scanner scanner{path, reader.fromConfiguration(getResourcePath("configuration/scanner.lex"))};
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
    // Non-keyword still id
    bool sawX = false;
    for (const auto &t : toks) {
        if (t.lexeme == "x") {
            EXPECT_EQ(t.id, "id");
            sawX = true;
        }
    }
    EXPECT_TRUE(sawX);
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

} // namespace
