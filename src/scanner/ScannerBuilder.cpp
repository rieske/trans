#include "ScannerBuilder.h"

namespace scanner {

std::string ScannerBuilder::addState(std::unique_ptr<State> newStatte) {
    State* state = namedStates.insert({newStatte->getName(), std::move(newStatte)}).first->second.get();
    if (!startState) {
        startState = state;
    }
    return state->getName();
}

void ScannerBuilder::addTransition(std::string fromState, std::string transitionOn, std::string transitionTo) {
    namedStateTransitions[fromState].push_back(std::make_pair(transitionOn, transitionTo));
}

void ScannerBuilder::addKeyword(std::string keyword) {
    keywordIds[keyword] = nextKeywordId++;
}

FiniteAutomaton* ScannerBuilder::build() {
    for (auto& namedState : namedStates) {
        auto& state = namedState.second;
        auto transitionsAtState = namedStateTransitions.find(state->getName());
        if (transitionsAtState != namedStateTransitions.end()) {
            for (auto& namedTransition : transitionsAtState->second) {
                state->addTransition(namedTransition.first, namedStates.at(namedTransition.second).get());
            }
        }
    }

    return new FiniteAutomaton{startState, keywordIds, std::move(namedStates)};
}

} // namespace scanner

