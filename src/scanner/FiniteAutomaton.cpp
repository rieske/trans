#include "FiniteAutomaton.h"

namespace scanner {

FiniteAutomaton::FiniteAutomaton(
        State* startState,
        std::map<std::string, unsigned> keywordIds,
        std::map<std::string, std::unique_ptr<State>> namedStates
):
    startState { startState },
    currentState { startState },
    keywordIds { keywordIds },
    namedStates { std::move(namedStates) }
{
}

void FiniteAutomaton::updateState(char inputSymbol) {
    accumulatedLexeme.clear();
    accumulatedToken.clear();

    auto nextState = currentState->nextStateForCharacter(inputSymbol);
    if (nextState->isFinal()) {
        accumulatedToken = currentState->getTokenId();
        if (currentState->needsKeywordLookup() && (keywordIds.find(accumulator) != keywordIds.end())) {
            accumulatedToken = accumulator;
        }
        if (!accumulatedToken.empty()) {
            accumulatedLexeme = accumulator;
        }
        accumulator.clear();
        currentState = startState->nextStateForCharacter(inputSymbol);
    } else {
        currentState = nextState;
    }

    if (currentState != startState) {
        accumulator += inputSymbol;
    }
}

bool FiniteAutomaton::isAtFinalState() const {
    return !accumulatedToken.empty();
}

std::string FiniteAutomaton::getAccumulatedLexeme() const {
    return accumulatedLexeme;
}

std::string FiniteAutomaton::getAccumulatedToken() const {
    return accumulatedToken;
}

} // namespace scanner

