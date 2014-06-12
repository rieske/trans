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

	Token token { "", "", 0 };

	ASSERT_THAT(scanner.nextToken(), tokenMatches("int", "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "MAXLINE"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("=", "="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("int_const", "255"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(";", ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("int", "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "write_out"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("(", "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("char", "char"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("*", "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "type"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(",", ","));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("void", "void"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("*", "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "out"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(")", ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("{", "{"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("if", "if"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("(", "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "type"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("==", "=="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("string", "\"%s\""));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(")", ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("{", "{"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("char", "char"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("*", "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "string"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("=", "="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("(", "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("char", "char"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("*", "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(")", ")"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "out"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(";", ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("while", "while"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("(", "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("*", "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "string"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("!=", "!="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("literal", "'\\0'"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(")", ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("output", "output"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("*", "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "string"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("++", "++"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(";", ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("return", "return"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("int_const", "1"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(";", ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("}", "}"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("else", "else"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("if", "if"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("(", "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "type"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("==", "=="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("string", "\"%d\""));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(")", ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("{", "{"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("int", "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("*", "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "integer"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("=", "="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("(", "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("int", "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("*", "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(")", ")"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "out"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(";", ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("output", "output"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("*", "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "integer"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(";", ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("return", "return"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("int_const", "1"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(";", ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("}", "}"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("return", "return"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("int_const", "0"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(";", ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("}", "}"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("int", "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "getLine"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("(", "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("char", "char"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("*", "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "line"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(")", ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("{", "{"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("char", "char"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "c"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(";", ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("int", "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "i"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("=", "="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("int_const", "0"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(";", ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("while", "while"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("(", "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("(", "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "c"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("!=", "!="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("literal", "'\\n'"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(")", ")"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("&&", "&&"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("(", "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "i"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("<", "<"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("id", "MAXLINE"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("-", "-"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches("int_const", "2"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(")", ")"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(")", ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches("{", "{"));

	/*
	 input 12
	 c 25
	 ; 20
	 * 24
	 line 25
	 ++ 36
	 = 21
	 c 25
	 ; 20
	 ++ 36
	 i 25
	 ; 20
	 } 23
	 * 24
	 line 25
	 ++ 36
	 = 21
	 '\n' 39
	 ; 20
	 * 24
	 line 25
	 = 21
	 '\0' 39
	 ; 20
	 if 5
	 ( 26
	 i 25
	 == 41
	 MAXLINE 25
	 ) 27
	 return 11
	 0 38
	 ; 20
	 else 6
	 return 11
	 1 38
	 ; 20
	 } 23
	 int 1
	 countSomething 25
	 ( 26
	 ) 27
	 { 22
	 int 1
	 a 25
	 , 28
	 b 25
	 , 28
	 c 25
	 , 28
	 d 25
	 ; 20
	 a 25
	 = 21
	 2 38
	 ; 20
	 b 25
	 = 21
	 5 38
	 ; 20
	 c 25
	 = 21
	 9 38
	 ; 20
	 d 25
	 = 21
	 1 38
	 ; 20
	 return 11
	 - 60
	 ( 26
	 a 25
	 * 24
	 2 38
	 - 60
	 5 38
	 * 24
	 d 25
	 + 44
	 a 25
	 - 60
	 d 25
	 * 24
	 - 60
	 b 25
	 - 60
	 b 25
	 - 60
	 b 25
	 ) 27
	 * 24
	 2 38
	 + 44
	 c 25
	 ; 20
	 } 23
	 int 1
	 main 25
	 ( 26
	 ) 27
	 { 22
	 char 2
	 * 24
	 str 25
	 = 21
	 "Hello World!\n" 40
	 ; 20
	 char 2
	 c 25
	 = 21
	 'c' 39
	 ; 20
	 char 2
	 nl 25
	 = 21
	 '\n' 39
	 ; 20
	 char 2
	 lineIn 25
	 [ 29
	 MAXLINE 25
	 ] 30
	 ; 20
	 int 1
	 retVal 25
	 ; 20
	 retVal 25
	 = 21
	 write_out 25
	 ( 26
	 "%s" 40
	 , 28
	 str 25
	 ) 27
	 ; 20
	 write_out 25
	 ( 26
	 "%d" 40
	 , 28
	 retVal 25
	 ) 27
	 ; 20
	 if 5
	 ( 26
	 getLine 25
	 ( 26
	 lineIn 25
	 ) 27
	 ) 27
	 write_out 25
	 ( 26
	 "%s" 40
	 , 28
	 lineIn 25
	 ) 27
	 ; 20
	 write_out 25
	 ( 26
	 "%d" 40
	 , 28
	 countSomething 25
	 ( 26
	 ) 27
	 ) 27
	 ; 20
	 return 11
	 0 38
	 ; 20
	 } 23
	 0
	 */
}
