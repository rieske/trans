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

	Token token { 0, "" };

	ASSERT_THAT(scanner.nextToken(), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "MAXLINE"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(21, "="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(38, "255"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "write_out"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(26, "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(2, "char"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "type"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(28, ","));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(3, "void"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "out"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(22, "{"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(5, "if"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(26, "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "type"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(41, "=="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(40, "\"%s\""));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(22, "{"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(2, "char"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "string"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(21, "="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(26, "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(2, "char"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(27, ")"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "out"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(7, "while"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(26, "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "string"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(55, "!="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(39, "'\\0'"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(13, "output"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "string"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(36, "++"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(11, "return"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(38, "1"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(23, "}"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(6, "else"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(5, "if"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(26, "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "type"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(41, "=="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(40, "\"%d\""));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(22, "{"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "integer"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(21, "="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(26, "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(27, ")"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "out"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(13, "output"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "integer"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(11, "return"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(38, "1"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(23, "}"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(11, "return"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(38, "0"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(23, "}"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "getLine"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(26, "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(2, "char"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "line"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(22, "{"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(2, "char"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "c"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "i"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(21, "="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(38, "0"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(7, "while"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(26, "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(26, "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "c"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(55, "!="));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(39, "'\\n'"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(27, ")"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(32, "&&"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(26, "("));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "i"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(56, "<"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(25, "MAXLINE"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(60, "-"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(38, "2"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(27, ")"));
	ASSERT_THAT(scanner.nextToken(), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.nextToken(), tokenMatches(22, "{"));

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
