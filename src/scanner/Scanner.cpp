#include "Scanner.h"

namespace scanner {

Scanner::Scanner(std::string fileName, std::unique_ptr<FiniteAutomaton> stateMachine, LexicalSession& session) :
        translationUnit { fileName },
        automaton { std::move(stateMachine) },
        session_ { session } {
    automaton->setSession(&session_);
}

Token Scanner::nextToken() {
    char currentCharacter { '\0' };
    translation_unit::Context tokenStart = translationUnit.getContext();
    try {
        do {
            const bool wasAtStart = automaton->isAtStartState();
            if (wasAtStart) {
                tokenStart = translationUnit.getContext();
            }
            currentCharacter = translationUnit.getNextCharacter();
            automaton->updateState(currentCharacter);
            if (automaton->isAtFinalState()) {
                break;
            }
        } while (currentCharacter != '\0');
    } catch (const std::runtime_error& error) {
        throw LexError { tokenStart, error.what() };
    }
    return { automaton->getAccumulatedToken(), automaton->getAccumulatedLexeme(), tokenStart };
}

} // namespace scanner
