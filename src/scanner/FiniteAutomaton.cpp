#include "FiniteAutomaton.h"
#include "LexicalSession.h"

namespace scanner {

FiniteAutomaton::FiniteAutomaton(
        State* startState,
        std::unordered_set<std::string> keywords,
        std::map<std::string, std::unique_ptr<State>> namedStates
):
    startState { startState },
    currentState { startState },
    keywords { std::move(keywords) },
    namedStates { std::move(namedStates) }
{
    accumulator.reserve(64);
}

void FiniteAutomaton::updateState(char inputSymbol) {
    auto nextState = currentState->nextStateForCharacter(inputSymbol);
    if (nextState->isFinal()) {
        accumulatedToken = currentState->getTokenId();
        if (currentState->needsKeywordLookup()) {
            if (keywords.contains(accumulator)) {
                accumulatedToken = accumulator;
            } else if (isTypedefName(accumulator)) {
                accumulatedToken = "typedef_name";
            }
        }
        if (!accumulatedToken.empty()) {
            accumulatedLexeme = accumulator;
        } else {
            accumulatedLexeme.clear();
        }
        accumulator.clear();
        currentState = startState->nextStateForCharacter(inputSymbol);
    } else {
        accumulatedToken.clear();
        accumulatedLexeme.clear();
        currentState = nextState;
    }

    if (currentState != startState) {
        accumulator += inputSymbol;
    }
}

bool FiniteAutomaton::isAtStartState() const {
    return currentState == startState;
}

bool FiniteAutomaton::isAtFinalState() const {
    return !accumulatedToken.empty();
}

const std::string& FiniteAutomaton::getAccumulatedLexeme() const {
    return accumulatedLexeme;
}

const std::string& FiniteAutomaton::getAccumulatedToken() const {
    return accumulatedToken;
}

bool FiniteAutomaton::isTypedefName(const std::string& name) const {
    return session_ && session_->isTypedef(name);
}

} // namespace scanner
