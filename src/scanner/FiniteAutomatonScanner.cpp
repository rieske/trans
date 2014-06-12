#include "FiniteAutomatonScanner.h"

#include "../driver/TranslationUnit.h"
#include "StateMachine.h"
#include "StateMachineFactory.h"
#include "Token.h"

FiniteAutomatonScanner::FiniteAutomatonScanner(TranslationUnit* translationUnit, StateMachineFactory* stateMachineFactory) :
		translationUnit { translationUnit },
		automaton { stateMachineFactory->createAutomaton() } {
}

FiniteAutomatonScanner::~FiniteAutomatonScanner() {
}

Token FiniteAutomatonScanner::nextToken() {
	char currentCharacter;
	do {
		currentCharacter = translationUnit->getNextCharacter();
		automaton->updateState(currentCharacter);
	} while (!automaton->isAtFinalState() && currentCharacter != '\0');
	return {automaton->getAccumulatedToken(), automaton->getAccumulatedLexeme(), translationUnit->getCurrentLineNumber()};
}
