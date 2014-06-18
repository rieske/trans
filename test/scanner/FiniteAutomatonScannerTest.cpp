#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/Token.h"
#include "scanner/StateMachineFactory.h"
#include "scanner/FiniteAutomatonFactory.h"
#include "scanner/StateMachine.h"
#include "scanner/FiniteAutomatonScanner.h"
#include "driver/TranslationUnit.h"
#include "driver/TranslationUnit.h"
#include "TokenMatcher.h"
#include <memory>

using namespace testing;

TEST(FiniteAutomatonScannerTest, scansTheExampleProgram) {
	StateMachineFactory* stateMachineFactory { new FiniteAutomatonFactory("resources/configuration/scanner.lex") };
	FiniteAutomatonScanner scanner { new TranslationUnit { "test/programs/example_prog.src" }, stateMachineFactory };

	/*int line { };
	 std::string id = "'$end$'";
	 do {
	 Token tok { scanner.nextToken() };
	 id = tok.id;
	 if (line != tok.line) {
	 std::cerr << "\n";
	 }
	 line = tok.line;
	 std::cerr << "ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {\"" << tok.id << "\", \"" << tok.lexeme << "\", " << tok.line
	 << "}));\n";
	 } while (id != "'$end$'");*/

	Token token { "", "", 0 };

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int", "int", 2 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "MAXLINE", 2 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 2 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "255", 2 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 3 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int", "int", 5 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "write_out", 5 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 5 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "char", "char", 5 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 5 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "type", 5 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ",", ",", 5 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "void", "void", 5 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 5 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "out", 5 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 6 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "{", "{", 7 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "if", "if", 7 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 7 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "type", 7 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "==", "==", 7 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "string", "\"%s\"", 7 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 7 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "{", "{", 9 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "char", "char", 9 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 9 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "string", 9 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 9 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 9 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "char", "char", 9 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 9 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 9 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "out", 9 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 10 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "while", "while", 10 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 10 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 10 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "string", 10 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "!=", "!=", 10 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "literal", "'\\0'", 10 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 11 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "output", "output", 11 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 11 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "string", 11 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "++", "++", 11 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 12 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "return", "return", 12 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "1", 12 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 13 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "}", "}", 14 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "else", "else", 14 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "if", "if", 14 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 14 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "type", 14 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "==", "==", 14 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "string", "\"%d\"", 14 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 15 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "{", "{", 16 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int", "int", 16 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 16 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "integer", 16 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 16 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 16 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int", "int", 16 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 16 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 16 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "out", 16 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 17 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "output", "output", 17 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 17 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "integer", 17 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 18 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "return", "return", 18 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "1", 18 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 19 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "}", "}", 20 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "return", "return", 20 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "0", 20 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 21 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "}", "}", 22 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int", "int", 23 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "getLine", 23 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 23 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "char", "char", 23 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 23 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "line", 23 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 24 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "{", "{", 25 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "char", "char", 25 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "c", 25 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 26 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int", "int", 26 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "i", 26 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 26 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "0", 26 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "while", "while", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "c", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "!=", "!=", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "literal", "'\\n'", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "&&", "&&", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "i", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "<", "<", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "MAXLINE", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "-", "-", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "2", 27 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 27 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 28 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "{", "{", 29 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "input", "input", 29 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "c", 29 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 30 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 30 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "line", 30 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "++", "++", 30 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 30 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "c", 30 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 31 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "++", "++", 31 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "i", 31 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 32 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "}", "}", 33 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 34 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "line", 34 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "++", "++", 34 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 34 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "literal", "'\\n'", 34 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 35 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 35 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "line", 35 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 35 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "literal", "'\\0'", 35 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 36 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "if", "if", 37 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 37 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "i", 37 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "==", "==", 37 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "MAXLINE", 37 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 38 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "return", "return", 38 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "0", 38 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 39 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "else", "else", 40 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "return", "return", 40 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "1", 40 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 41 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "}", "}", 42 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int", "int", 43 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "countSomething", 43 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 43 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 44 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "{", "{", 45 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int", "int", 45 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "a", 45 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ",", ",", 45 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "b", 45 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ",", ",", 45 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "c", 45 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ",", ",", 45 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "d", 45 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 46 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "a", 46 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 46 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "2", 46 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 47 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "b", 47 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 47 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "5", 47 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 48 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "c", 48 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 48 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "9", 48 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 49 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "d", 49 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 49 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "1", 49 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "return", "return", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "-", "-", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "a", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "2", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "-", "-", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "5", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "d", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "+", "+", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "a", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "-", "-", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "d", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "-", "-", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "b", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "-", "-", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "b", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "-", "-", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "b", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "2", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "+", "+", 50 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "c", 50 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 51 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "}", "}", 52 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int", "int", 53 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "main", 53 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 53 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 54 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "{", "{", 55 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "char", "char", 55 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "*", "*", 55 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "str", 55 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 55 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "string", "\"Hello World!\\n\"", 55 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 56 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "char", "char", 56 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "c", 56 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 56 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "literal", "'c'", 56 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 57 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "char", "char", 57 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "nl", 57 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 57 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "literal", "'\\n'", 57 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 58 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "char", "char", 59 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "lineIn", 59 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "[", "[", 59 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "MAXLINE", 59 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "]", "]", 59 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 60 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int", "int", 61 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "retVal", 61 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 62 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "retVal", 63 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "=", "=", 63 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "write_out", 63 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 63 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "string", "\"%s\"", 63 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ",", ",", 63 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "str", 63 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 63 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 63 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "write_out", 64 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 64 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "string", "\"%d\"", 64 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ",", ",", 64 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "retVal", 64 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 64 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 65 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "if", "if", 66 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 66 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "getLine", 66 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 66 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "lineIn", 66 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 66 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 67 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "write_out", 67 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 67 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "string", "\"%s\"", 67 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ",", ",", 67 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "lineIn", 67 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 67 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 68 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "write_out", 69 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 69 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token {"string", "\"%d\"", 69}));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ",", ",", 69 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "id", "countSomething", 69 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "(", "(", 69 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 69 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ")", ")", 69 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 70 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "return", "return", 70 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "int_const", "0", 70 }));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { ";", ";", 71 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "}", "}", 71 }));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(Token { "'$end$'", "", 71 }));
}
