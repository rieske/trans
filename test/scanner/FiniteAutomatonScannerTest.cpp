#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/StateMachineFactory.h"
#include "scanner/FiniteAutomatonFactory.h"
#include "scanner/StateMachine.h"
#include "scanner/FiniteAutomatonScanner.h"
#include "driver/TranslationUnit.h"
#include "driver/SourceTranslationUnit.h"
#include "TokenMatcher.h"
#include <memory>

using namespace testing;

class MockStateMachineFactory: public StateMachineFactory {
public:
	std::unique_ptr<StateMachine> createAutomaton() const {
		return std::unique_ptr<StateMachine> { createAutomatonProxy() };
	}

	MOCK_CONST_METHOD0(createAutomatonProxy, StateMachine* ());
};

class MockStateMachine: public StateMachine {
public:
	MOCK_METHOD1(updateState, void (char inputSymbol));
	MOCK_CONST_METHOD0(isAtFinalState, bool ());
	MOCK_METHOD0(getCurrentToken, Token ());
};

class MockTranslationUnit: public TranslationUnit {
public:
	MOCK_CONST_METHOD0(getFileName, std::string ());
	MOCK_METHOD0(getNextToken, Token ());
	MOCK_METHOD0(getNextCharacter, char ());
};

TEST(FiniteAutomatonScanner, usesStateMachineToProcessTranslationUnitCharacters) {
	std::unique_ptr<StrictMock<MockStateMachineFactory>> stateMachineFactoryPtr { new StrictMock<MockStateMachineFactory> };
	StrictMock<MockStateMachineFactory> *stateMachineFactory = stateMachineFactoryPtr.get();

	StrictMock<MockStateMachine>* stateMachine = new StrictMock<MockStateMachine>;
	Token expectedToken = Token { 0, "expected" };

	StrictMock<MockTranslationUnit> translationUnit;
	{
		InSequence sequence;
		EXPECT_CALL(*stateMachineFactory, createAutomatonProxy()).WillOnce(Return(stateMachine));

		EXPECT_CALL(translationUnit, getNextCharacter()).WillOnce(Return('a'));
		EXPECT_CALL(*stateMachine, updateState('a'));
		EXPECT_CALL(*stateMachine, isAtFinalState()).WillOnce(Return(false));

		EXPECT_CALL(translationUnit, getNextCharacter()).WillOnce(Return('b'));
		EXPECT_CALL(*stateMachine, updateState('b'));
		EXPECT_CALL(*stateMachine, isAtFinalState()).WillOnce(Return(true));

		EXPECT_CALL(*stateMachine, getCurrentToken()).WillOnce(Return(expectedToken));
	}
	FiniteAutomatonScanner scanner { std::move(stateMachineFactoryPtr) };

	Token token = scanner.scan(translationUnit);

	ASSERT_THAT(token, tokenMatches(expectedToken));
}

TEST(FiniteAutomatonScannerIntegrationTest, scansTheExampleProgram) {
	std::unique_ptr<FiniteAutomatonFactory> stateMachineFactory { new FiniteAutomatonFactory("resources/configuration/scanner.lex") };
	FiniteAutomatonScanner scanner { std::move(stateMachineFactory) };

	SourceTranslationUnit translationUnit("test/programs/example_prog.src", scanner);

	Token token { 0, "" };

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "MAXLINE"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(21, "="));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(38, "255"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "write_out"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(26, "("));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(2, "char"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "type"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(28, ","));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(3, "void"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "out"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(22, "{"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(5, "if"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(26, "("));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "type"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(41, "=="));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(40, "\"%s\""));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(22, "{"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(2, "char"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "string"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(21, "="));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(26, "("));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(2, "char"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(27, ")"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "out"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(7, "while"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(26, "("));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "string"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(55, "!="));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(39, "'\\0'"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(13, "output"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "string"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(36, "++"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(11, "return"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(38, "1"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(23, "}"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(6, "else"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(5, "if"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(26, "("));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "type"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(41, "=="));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(40, "\"%d\""));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(22, "{"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "integer"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(21, "="));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(26, "("));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(27, ")"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "out"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(13, "output"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "integer"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(11, "return"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(38, "1"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(23, "}"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(11, "return"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(38, "0"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(23, "}"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "getLine"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(26, "("));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(2, "char"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(24, "*"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "line"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(22, "{"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(2, "char"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "c"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(1, "int"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "i"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(21, "="));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(38, "0"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(20, ";"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(7, "while"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(26, "("));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(26, "("));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "c"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(55, "!="));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(39, "'\\n'"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(27, ")"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(32, "&&"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(26, "("));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "i"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(56, "<"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(25, "MAXLINE"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(60, "-"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(38, "2"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(27, ")"));
	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(27, ")"));

	ASSERT_THAT(scanner.scan(translationUnit), tokenMatches(22, "{"));

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
