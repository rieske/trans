#include "FiniteAutomatonScanner.h"

#include "Token.h"

namespace scanner {

FiniteAutomatonScanner::FiniteAutomatonScanner(TranslationUnit* translationUnit, FiniteAutomaton* stateMachine) :
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
    return {automaton->getAccumulatedToken(), automaton->getAccumulatedLexeme(), translationUnit->getContext()};
}

} // namespace scanner

