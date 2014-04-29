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



	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.getLexeme() << " " << token.getId() << std::endl;
}
