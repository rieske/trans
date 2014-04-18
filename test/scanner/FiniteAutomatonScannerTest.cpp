#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "scanner/StateMachineFactory.h"
#include "scanner/FiniteAutomatonFactory.h"
#include "scanner/StateMachine.h"
#include "scanner/FiniteAutomatonScanner.h"
#include "driver/TranslationUnit.h"
#include "driver/SourceTranslationUnit.h"
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
	FiniteAutomatonScanner scanner { std::move(stateMachineFactoryPtr) };

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

	Token token = scanner.scan(translationUnit);

	ASSERT_THAT(token.value, Eq(expectedToken.value));
}

TEST(FiniteAutomatonScanner, throwsRuntimeErrorWhenFinalStateIsNotReached) {
	std::unique_ptr<StrictMock<MockStateMachineFactory>> stateMachineFactoryPtr { new StrictMock<MockStateMachineFactory> };
	StrictMock<MockStateMachineFactory> *stateMachineFactory = stateMachineFactoryPtr.get();
	FiniteAutomatonScanner scanner { std::move(stateMachineFactoryPtr) };

	StrictMock<MockStateMachine>* stateMachine = new StrictMock<MockStateMachine>;
	EXPECT_CALL(*stateMachine, updateState('\0'));
	EXPECT_CALL(*stateMachine, isAtFinalState()).WillOnce(Return(false));
	EXPECT_CALL(*stateMachineFactory, createAutomatonProxy()).WillOnce(Return(stateMachine));

	StrictMock<MockTranslationUnit> translationUnit;
	EXPECT_CALL(translationUnit, getNextCharacter()).Times(2).WillRepeatedly(Return('\0'));

	ASSERT_THROW(scanner.scan(translationUnit), std::runtime_error);
}

TEST(FiniteAutomatonScannerIntegrationTest, scansTheExampleProgram) {
	std::unique_ptr<FiniteAutomatonFactory> stateMachineFactory { new FiniteAutomatonFactory("resources/configuration/scanner.lex") };
	FiniteAutomatonScanner scanner { std::move(stateMachineFactory) };

	SourceTranslationUnit translationUnit("test/programs/example_prog.src", scanner);

	Token token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
	token = scanner.scan(translationUnit);
	std::cerr << token.value << " " << token.type << std::endl;
}
