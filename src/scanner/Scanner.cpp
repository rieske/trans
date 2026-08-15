#include "Scanner.h"

namespace scanner {

Scanner::Scanner(std::string fileName, std::unique_ptr<FiniteAutomaton> stateMachine, LexicalSession& session) :
        translationUnit { fileName },
        automaton { std::move(stateMachine) },
        session_ { session } {
    automaton->setTypedefRegistry(&session_.typedefs);
}

Token Scanner::nextToken() {
    char currentCharacter { '\0' };
    translation_unit::Context tokenStart = translationUnit.getContext();
    bool haveTokenStart { false };
    do {
        const translation_unit::Context contextBefore = translationUnit.getContext();
        currentCharacter = translationUnit.getNextCharacter();
        const bool wasAtStart = automaton->isAtStartState();
        automaton->updateState(currentCharacter);
        if (automaton->isAtFinalState()) {
            if (!haveTokenStart) {
                tokenStart = contextBefore;
            }
            break;
        }
        if (automaton->isAtStartState()) {
            haveTokenStart = false;
        } else if (wasAtStart) {
            tokenStart = contextBefore;
            haveTokenStart = true;
        }
    } while (currentCharacter != '\0');
    return { automaton->getAccumulatedToken(), automaton->getAccumulatedLexeme(), tokenStart };
}

} // namespace scanner
