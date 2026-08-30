#include "State.h"

#include <stdexcept>

namespace scanner {

State::State(std::string stateName, std::string tokenId) :
        stateName { stateName },
        tokenId { tokenId },
        transitions {},
        hasTransition { false } {
}

State::~State() = default;

const std::string& State::getName() const {
    return stateName;
}

void State::addTransition(std::string charactersForTransition, State* state) {
    if (charactersForTransition.empty()) {
        for (State*& slot : transitions) {
            if (slot == nullptr) {
                slot = state;
            }
        }
    } else {
        for (char characterForTransition : charactersForTransition) {
            transitions[static_cast<unsigned char>(characterForTransition)] = state;
        }
    }
    hasTransition = true;
}

const State* State::nextStateForCharacter(char c) const {
    if (State* next = transitions[static_cast<unsigned char>(c)]) {
        return next;
    }
    throw std::runtime_error { "Can't reach next state for given input: " + std::string { c } };
}

const std::string& State::getTokenId() const {
    return tokenId;
}

bool State::needsKeywordLookup() const {
    return false;
}

bool State::isFinal() const {
    return !hasTransition;
}

IdentifierState::IdentifierState(std::string stateName, std::string tokenId): State { stateName, tokenId } {}
IdentifierState::~IdentifierState() = default;

bool IdentifierState::needsKeywordLookup() const {
    return true;
}

StringLiteralState::StringLiteralState(std::string stateName, std::string tokenId): State { stateName, tokenId } {}
StringLiteralState::~StringLiteralState() = default;

const State* StringLiteralState::nextStateForCharacter(char c) const {
    if (c == ' ') {
        return this;
    }
    if (c == '\n') {
        throw std::runtime_error("newline encountered in string literal");
    }
    return State::nextStateForCharacter(c);
}

EOLCommentState::EOLCommentState(std::string stateName): State { stateName, "" } {}
EOLCommentState::~EOLCommentState() = default;

const State* EOLCommentState::nextStateForCharacter(char c) const {
    return (c != '\n') ? this : State::nextStateForCharacter(c);
}

} // namespace scanner

