#include "Scanner.h"

#include "Token.h"

namespace scanner {

Scanner::Scanner(std::string fileName, FiniteAutomaton* stateMachine) :
        translationUnit { fileName },
        automaton { stateMachine } {
}

Token Scanner::nextToken() {
    char currentCharacter;
    do {
        currentCharacter = translationUnit.getNextCharacter();
        automaton->updateState(currentCharacter);
    } while (!automaton->isAtFinalState() && currentCharacter != '\0');
    return {automaton->getAccumulatedToken(), automaton->getAccumulatedLexeme(), translationUnit.getContext()};
}

} // namespace scanner

