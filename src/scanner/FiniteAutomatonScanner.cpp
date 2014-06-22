#include "FiniteAutomatonScanner.h"

#include <algorithm>

#include "../driver/TranslationUnit.h"
#include "StateMachine.h"
#include "Token.h"

FiniteAutomatonScanner::FiniteAutomatonScanner(TranslationUnit* translationUnit, StateMachine* stateMachine) :
		translationUnit { translationUnit },
		automaton { stateMachine } {
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
