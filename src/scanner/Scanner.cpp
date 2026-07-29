#include "Scanner.h"

#include "Token.h"

namespace scanner {

Scanner::Scanner(std::string fileName, std::unique_ptr<FiniteAutomaton> stateMachine, LexicalSession& session) :
        translationUnit { fileName },
        automaton { std::move(stateMachine) },
        session_ { session } {
    automaton->setTypedefRegistry(&session_.typedefs);
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
