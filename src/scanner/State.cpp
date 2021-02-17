#include "State.h"

namespace scanner {

State::State(std::string stateName, std::string tokenId) :
        stateName { stateName },
        tokenId { tokenId },
        wildcardTransition { nullptr } {
}

State::~State() {
}

std::string State::getName() const {
    return stateName;
}

void State::addTransition(std::string charactersForTransition, State* state) {
    if (charactersForTransition.empty()) {
        wildcardTransition = state;
    } else {
        for (char characterForTransition : charactersForTransition) {
            transitions[characterForTransition] = state;
        }
    }
}

const State* State::nextStateForCharacter(char c) const {
    auto stateAtCharacter = transitions.find(c);
    if (stateAtCharacter != transitions.end()) {
        return stateAtCharacter->second;
    }
    if (wildcardTransition != nullptr) {
        return wildcardTransition;
    }
    throw std::runtime_error { "Can't reach next state for given input: " + std::string { c } };
}

std::string State::getTokenId() const {
    return tokenId;
}

bool State::needsKeywordLookup() const {
    return false;
}

bool State::isFinal() const {
    return wildcardTransition == nullptr && transitions.empty();
}

std::ostream& operator<<(std::ostream& ostream, const State& state) {
    ostream << state.stateName << ":" << std::endl;
    for (const auto& transition : state.transitions) {
        ostream << "\t" << transition.first << "\t->\t" << transition.second->getName() << std::endl;
    }
    if (state.wildcardTransition != nullptr) {
        ostream << "\t<any character>\t->\t" << state.wildcardTransition->getName() << std::endl;
    }
    return ostream;
}

} // namespace scanner

