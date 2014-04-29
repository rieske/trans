#include "FiniteAutomatonScanner.h"

#include "../driver/TranslationUnit.h"
#include "StateMachine.h"
#include "StateMachineFactory.h"
#include "Token.h"

FiniteAutomatonScanner::FiniteAutomatonScanner(std::unique_ptr<StateMachineFactory> stateMachineFactory) {
	automaton = stateMachineFactory->createAutomaton();
}

FiniteAutomatonScanner::~FiniteAutomatonScanner() {
}

Token FiniteAutomatonScanner::scan(TranslationUnit& translationUnit) {
	char currentCharacter;
	do {
		currentCharacter = translationUnit.getNextCharacter();
		automaton->updateState(currentCharacter);
	} while (!automaton->isAtFinalState() && currentCharacter != '\0');
	return automaton->getCurrentToken();
}
