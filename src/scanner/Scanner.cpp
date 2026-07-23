#include "Scanner.h"

#include "Token.h"

namespace scanner {

Scanner::Scanner(std::string fileName, std::unique_ptr<FiniteAutomaton> stateMachine) :
        translationUnit { fileName },
        automaton { std::move(stateMachine) } {
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

