#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/LexFileScannerBuilder.h"
#include "scanner/Token.h"
#include "scanner/Scanner.h"
#include "TokenMatcher.h"
#include <memory>

#include "ResourceHelpers.h"

using namespace testing;
using namespace scanner;

TEST(ScannerTest, scansTheExampleProgram) {
    auto exampleProgramFilename = getTestResourcePath("programs/example_prog.src");
    LexFileScannerBuilder builder;
    Scanner scanner {
        exampleProgramFilename,
        builder.fromConfiguration(getResourcePath("configuration/scanner.lex"))
    };

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {exampleProgramFilename, 2} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "MAXLINE", {exampleProgramFilename, 2} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 2} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "255", {exampleProgramFilename, 2} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 3} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {exampleProgramFilename, 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "write_out", {exampleProgramFilename, 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {exampleProgramFilename, 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "type", {exampleProgramFilename, 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"void", "void", {exampleProgramFilename, 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "out", {exampleProgramFilename, 5} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 6} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {exampleProgramFilename, 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"if", "if", {exampleProgramFilename, 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "type", {exampleProgramFilename, 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"==", "==", {exampleProgramFilename, 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%s\"", {exampleProgramFilename, 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 7} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {exampleProgramFilename, 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {exampleProgramFilename, 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "string", {exampleProgramFilename, 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {exampleProgramFilename, 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "out", {exampleProgramFilename, 9} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"while", "while", {exampleProgramFilename, 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "string", {exampleProgramFilename, 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"!=", "!=", {exampleProgramFilename, 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char_const", "'\\0'", {exampleProgramFilename, 10} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "printf", {exampleProgramFilename, 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%d\"", {exampleProgramFilename, 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "string", {exampleProgramFilename, 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"++", "++", {exampleProgramFilename, 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 11} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 12} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {exampleProgramFilename, 12} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "1", {exampleProgramFilename, 12} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 13} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {exampleProgramFilename, 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"else", "else", {exampleProgramFilename, 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"if", "if", {exampleProgramFilename, 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "type", {exampleProgramFilename, 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"==", "==", {exampleProgramFilename, 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%d\"", {exampleProgramFilename, 14} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 15} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {exampleProgramFilename, 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {exampleProgramFilename, 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "integer", {exampleProgramFilename, 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {exampleProgramFilename, 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "out", {exampleProgramFilename, 16} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 17} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "printf", {exampleProgramFilename, 17} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 17} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%d\"", {exampleProgramFilename, 17} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 17} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 17} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "integer", {exampleProgramFilename, 17} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 17} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 18} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {exampleProgramFilename, 18} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "1", {exampleProgramFilename, 18} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 19} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {exampleProgramFilename, 20} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {exampleProgramFilename, 20} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "0", {exampleProgramFilename, 20} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 21} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {exampleProgramFilename, 22} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {exampleProgramFilename, 23} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "getLine", {exampleProgramFilename, 23} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 23} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {exampleProgramFilename, 23} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 23} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "line", {exampleProgramFilename, 23} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 24} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {exampleProgramFilename, 25} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {exampleProgramFilename, 25} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {exampleProgramFilename, 25} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 26} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {exampleProgramFilename, 26} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "i", {exampleProgramFilename, 26} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 26} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "0", {exampleProgramFilename, 26} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"while", "while", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"!=", "!=", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char_const", "'\\n'", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"&&", "&&", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "i", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"<", "<", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "MAXLINE", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "2", {exampleProgramFilename, 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 27} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 28} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {exampleProgramFilename, 29} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "scanf", {exampleProgramFilename, 29} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 29} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%d\"", {exampleProgramFilename, 29} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 29} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"&", "&", {exampleProgramFilename, 29} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {exampleProgramFilename, 29} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 29} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 30} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 30} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "line", {exampleProgramFilename, 30} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"++", "++", {exampleProgramFilename, 30} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 30} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {exampleProgramFilename, 30} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 31} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"++", "++", {exampleProgramFilename, 31} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "i", {exampleProgramFilename, 31} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 32} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {exampleProgramFilename, 33} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 34} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "line", {exampleProgramFilename, 34} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"++", "++", {exampleProgramFilename, 34} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 34} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char_const", "'\\n'", {exampleProgramFilename, 34} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 35} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 35} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "line", {exampleProgramFilename, 35} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 35} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char_const", "'\\0'", {exampleProgramFilename, 35} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 36} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"if", "if", {exampleProgramFilename, 37} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 37} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "i", {exampleProgramFilename, 37} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"==", "==", {exampleProgramFilename, 37} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "MAXLINE", {exampleProgramFilename, 37} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 38} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {exampleProgramFilename, 38} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "0", {exampleProgramFilename, 38} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 39} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"else", "else", {exampleProgramFilename, 40} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {exampleProgramFilename, 40} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "1", {exampleProgramFilename, 40} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 41} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {exampleProgramFilename, 42} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {exampleProgramFilename, 43} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "countSomething", {exampleProgramFilename, 43} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 43} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 44} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {exampleProgramFilename, 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {exampleProgramFilename, 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "a", {exampleProgramFilename, 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "b", {exampleProgramFilename, 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {exampleProgramFilename, 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "d", {exampleProgramFilename, 45} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 46} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "a", {exampleProgramFilename, 46} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 46} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "2", {exampleProgramFilename, 46} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 47} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "b", {exampleProgramFilename, 47} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 47} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "5", {exampleProgramFilename, 47} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 48} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {exampleProgramFilename, 48} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 48} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "9", {exampleProgramFilename, 48} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 49} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "d", {exampleProgramFilename, 49} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 49} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "1", {exampleProgramFilename, 49} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "a", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "2", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "5", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "d", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"+", "+", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "a", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "d", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "b", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "b", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "b", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "2", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"+", "+", {exampleProgramFilename, 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {exampleProgramFilename, 50} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 51} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {exampleProgramFilename, 52} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {exampleProgramFilename, 53} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "main", {exampleProgramFilename, 53} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 53} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 54} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {exampleProgramFilename, 55} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {exampleProgramFilename, 55} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {exampleProgramFilename, 55} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "str", {exampleProgramFilename, 55} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 55} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"Hello World!\\n\"", {exampleProgramFilename, 55} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 56} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {exampleProgramFilename, 56} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {exampleProgramFilename, 56} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 56} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char_const", "'c'", {exampleProgramFilename, 56} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 57} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {exampleProgramFilename, 57} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "nl", {exampleProgramFilename, 57} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 57} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char_const", "'\\n'", {exampleProgramFilename, 57} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 58} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {exampleProgramFilename, 59} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "lineIn", {exampleProgramFilename, 59} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"[", "[", {exampleProgramFilename, 59} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "MAXLINE", {exampleProgramFilename, 59} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"]", "]", {exampleProgramFilename, 59} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 60} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {exampleProgramFilename, 61} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "retVal", {exampleProgramFilename, 61} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 62} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "retVal", {exampleProgramFilename, 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {exampleProgramFilename, 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "write_out", {exampleProgramFilename, 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%s\"", {exampleProgramFilename, 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "str", {exampleProgramFilename, 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 63} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "write_out", {exampleProgramFilename, 64} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 64} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%d\"", {exampleProgramFilename, 64} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 64} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "retVal", {exampleProgramFilename, 64} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 64} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 65} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"if", "if", {exampleProgramFilename, 66} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 66} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "getLine", {exampleProgramFilename, 66} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 66} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "lineIn", {exampleProgramFilename, 66} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 66} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "write_out", {exampleProgramFilename, 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%s\"", {exampleProgramFilename, 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "lineIn", {exampleProgramFilename, 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 67} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 68} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "write_out", {exampleProgramFilename, 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%d\"", {exampleProgramFilename, 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {exampleProgramFilename, 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "countSomething", {exampleProgramFilename, 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {exampleProgramFilename, 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {exampleProgramFilename, 69} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 70} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {exampleProgramFilename, 70} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "0", {exampleProgramFilename, 70} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {exampleProgramFilename, 71} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {exampleProgramFilename, 71} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {Token::END, "", {exampleProgramFilename, 71} }));
}
