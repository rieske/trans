#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/Token.h"
#include "scanner/LexFileFiniteAutomaton.h"
#include "scanner/FiniteAutomatonScanner.h"
#include "driver/TranslationUnit.h"
#include "driver/TranslationUnit.h"
#include "TokenMatcher.h"
#include <memory>

using namespace testing;

TEST(FiniteAutomatonScannerTest, scansTheExampleProgram) {
    FiniteAutomatonScanner scanner { new TranslationUnit { "test/programs/example_prog.src" }, new LexFileFiniteAutomaton("resources/configuration/scanner.lex") };

   /* std::size_t line { };
    std::string id = "'$end$'";
    do {
        Token tok { scanner.nextToken() };
        id = tok.id;
        if (line != tok.context.getOffset()) {
            std::cerr << "\n";
        }
        line = tok.context.getOffset();
        std::cerr << "ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {\"" << tok.id << "\", \"" << tok.lexeme << "\", {\"" << tok.context.getSourceName() << "\", " << tok.context.getOffset()  << "} }));\n";
    } while (id != "'$end$'");*/


    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {"test/programs/example_prog.src", 2} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "MAXLINE", {"test/programs/example_prog.src", 2} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 2} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "255", {"test/programs/example_prog.src", 2} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 3} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {"test/programs/example_prog.src", 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "write_out", {"test/programs/example_prog.src", 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {"test/programs/example_prog.src", 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "type", {"test/programs/example_prog.src", 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {"test/programs/example_prog.src", 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"void", "void", {"test/programs/example_prog.src", 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 5} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "out", {"test/programs/example_prog.src", 5} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 6} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {"test/programs/example_prog.src", 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"if", "if", {"test/programs/example_prog.src", 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "type", {"test/programs/example_prog.src", 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"==", "==", {"test/programs/example_prog.src", 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%s\"", {"test/programs/example_prog.src", 7} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 7} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {"test/programs/example_prog.src", 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {"test/programs/example_prog.src", 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "string", {"test/programs/example_prog.src", 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {"test/programs/example_prog.src", 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 9} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "out", {"test/programs/example_prog.src", 9} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"while", "while", {"test/programs/example_prog.src", 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "string", {"test/programs/example_prog.src", 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"!=", "!=", {"test/programs/example_prog.src", 10} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"literal", "'\\0'", {"test/programs/example_prog.src", 10} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"output", "output", {"test/programs/example_prog.src", 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "string", {"test/programs/example_prog.src", 11} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"++", "++", {"test/programs/example_prog.src", 11} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 12} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {"test/programs/example_prog.src", 12} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "1", {"test/programs/example_prog.src", 12} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 13} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {"test/programs/example_prog.src", 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"else", "else", {"test/programs/example_prog.src", 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"if", "if", {"test/programs/example_prog.src", 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "type", {"test/programs/example_prog.src", 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"==", "==", {"test/programs/example_prog.src", 14} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%d\"", {"test/programs/example_prog.src", 14} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 15} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {"test/programs/example_prog.src", 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {"test/programs/example_prog.src", 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "integer", {"test/programs/example_prog.src", 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {"test/programs/example_prog.src", 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 16} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "out", {"test/programs/example_prog.src", 16} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 17} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"output", "output", {"test/programs/example_prog.src", 17} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 17} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "integer", {"test/programs/example_prog.src", 17} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 18} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {"test/programs/example_prog.src", 18} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "1", {"test/programs/example_prog.src", 18} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 19} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {"test/programs/example_prog.src", 20} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {"test/programs/example_prog.src", 20} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "0", {"test/programs/example_prog.src", 20} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 21} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {"test/programs/example_prog.src", 22} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {"test/programs/example_prog.src", 23} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "getLine", {"test/programs/example_prog.src", 23} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 23} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {"test/programs/example_prog.src", 23} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 23} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "line", {"test/programs/example_prog.src", 23} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 24} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {"test/programs/example_prog.src", 25} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {"test/programs/example_prog.src", 25} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {"test/programs/example_prog.src", 25} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 26} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {"test/programs/example_prog.src", 26} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "i", {"test/programs/example_prog.src", 26} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 26} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "0", {"test/programs/example_prog.src", 26} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"while", "while", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"!=", "!=", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"literal", "'\\n'", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"&&", "&&", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "i", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"<", "<", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "MAXLINE", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "2", {"test/programs/example_prog.src", 27} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 27} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 28} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {"test/programs/example_prog.src", 29} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"input", "input", {"test/programs/example_prog.src", 29} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {"test/programs/example_prog.src", 29} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 30} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 30} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "line", {"test/programs/example_prog.src", 30} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"++", "++", {"test/programs/example_prog.src", 30} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 30} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {"test/programs/example_prog.src", 30} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 31} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"++", "++", {"test/programs/example_prog.src", 31} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "i", {"test/programs/example_prog.src", 31} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 32} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {"test/programs/example_prog.src", 33} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 34} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "line", {"test/programs/example_prog.src", 34} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"++", "++", {"test/programs/example_prog.src", 34} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 34} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"literal", "'\\n'", {"test/programs/example_prog.src", 34} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 35} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 35} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "line", {"test/programs/example_prog.src", 35} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 35} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"literal", "'\\0'", {"test/programs/example_prog.src", 35} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 36} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"if", "if", {"test/programs/example_prog.src", 37} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 37} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "i", {"test/programs/example_prog.src", 37} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"==", "==", {"test/programs/example_prog.src", 37} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "MAXLINE", {"test/programs/example_prog.src", 37} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 38} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {"test/programs/example_prog.src", 38} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "0", {"test/programs/example_prog.src", 38} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 39} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"else", "else", {"test/programs/example_prog.src", 40} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {"test/programs/example_prog.src", 40} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "1", {"test/programs/example_prog.src", 40} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 41} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {"test/programs/example_prog.src", 42} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {"test/programs/example_prog.src", 43} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "countSomething", {"test/programs/example_prog.src", 43} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 43} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 44} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {"test/programs/example_prog.src", 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {"test/programs/example_prog.src", 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "a", {"test/programs/example_prog.src", 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {"test/programs/example_prog.src", 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "b", {"test/programs/example_prog.src", 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {"test/programs/example_prog.src", 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {"test/programs/example_prog.src", 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {"test/programs/example_prog.src", 45} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "d", {"test/programs/example_prog.src", 45} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 46} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "a", {"test/programs/example_prog.src", 46} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 46} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "2", {"test/programs/example_prog.src", 46} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 47} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "b", {"test/programs/example_prog.src", 47} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 47} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "5", {"test/programs/example_prog.src", 47} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 48} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {"test/programs/example_prog.src", 48} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 48} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "9", {"test/programs/example_prog.src", 48} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 49} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "d", {"test/programs/example_prog.src", 49} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 49} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "1", {"test/programs/example_prog.src", 49} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "a", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "2", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "5", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "d", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"+", "+", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "a", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "d", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "b", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "b", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"-", "-", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "b", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "2", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"+", "+", {"test/programs/example_prog.src", 50} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {"test/programs/example_prog.src", 50} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 51} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {"test/programs/example_prog.src", 52} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {"test/programs/example_prog.src", 53} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "main", {"test/programs/example_prog.src", 53} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 53} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 54} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"{", "{", {"test/programs/example_prog.src", 55} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {"test/programs/example_prog.src", 55} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"*", "*", {"test/programs/example_prog.src", 55} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "str", {"test/programs/example_prog.src", 55} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 55} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"Hello World!\\n\"", {"test/programs/example_prog.src", 55} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 56} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {"test/programs/example_prog.src", 56} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "c", {"test/programs/example_prog.src", 56} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 56} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"literal", "'c'", {"test/programs/example_prog.src", 56} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 57} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {"test/programs/example_prog.src", 57} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "nl", {"test/programs/example_prog.src", 57} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 57} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"literal", "'\\n'", {"test/programs/example_prog.src", 57} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 58} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"char", "char", {"test/programs/example_prog.src", 59} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "lineIn", {"test/programs/example_prog.src", 59} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"[", "[", {"test/programs/example_prog.src", 59} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "MAXLINE", {"test/programs/example_prog.src", 59} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"]", "]", {"test/programs/example_prog.src", 59} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 60} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int", "int", {"test/programs/example_prog.src", 61} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "retVal", {"test/programs/example_prog.src", 61} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 62} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "retVal", {"test/programs/example_prog.src", 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"=", "=", {"test/programs/example_prog.src", 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "write_out", {"test/programs/example_prog.src", 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%s\"", {"test/programs/example_prog.src", 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {"test/programs/example_prog.src", 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "str", {"test/programs/example_prog.src", 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 63} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 63} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "write_out", {"test/programs/example_prog.src", 64} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 64} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%d\"", {"test/programs/example_prog.src", 64} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {"test/programs/example_prog.src", 64} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "retVal", {"test/programs/example_prog.src", 64} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 64} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 65} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"if", "if", {"test/programs/example_prog.src", 66} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 66} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "getLine", {"test/programs/example_prog.src", 66} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 66} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "lineIn", {"test/programs/example_prog.src", 66} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 66} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "write_out", {"test/programs/example_prog.src", 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%s\"", {"test/programs/example_prog.src", 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {"test/programs/example_prog.src", 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "lineIn", {"test/programs/example_prog.src", 67} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 67} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 68} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "write_out", {"test/programs/example_prog.src", 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%d\"", {"test/programs/example_prog.src", 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {",", ",", {"test/programs/example_prog.src", 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"id", "countSomething", {"test/programs/example_prog.src", 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"(", "(", {"test/programs/example_prog.src", 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 69} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {")", ")", {"test/programs/example_prog.src", 69} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 70} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"return", "return", {"test/programs/example_prog.src", 70} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"int_const", "0", {"test/programs/example_prog.src", 70} }));

    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {";", ";", {"test/programs/example_prog.src", 71} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"}", "}", {"test/programs/example_prog.src", 71} }));
    ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"'$end$'", "", {"test/programs/example_prog.src", 71} }));
}
