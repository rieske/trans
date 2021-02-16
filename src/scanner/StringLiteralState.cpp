#include "StringLiteralState.h"

StringLiteralState::StringLiteralState(std::string stateName, std::string tokenId) :
    State { stateName, tokenId } {
}

StringLiteralState::~StringLiteralState() {
}

const State* StringLiteralState::nextStateForCharacter(char c) const {
    if (c == ' ') {
        return this;
    }
    if (c == '\n') {
        throw std::runtime_error("newline encountered in string literal");
    }
    return State::nextStateForCharacter(c);
}

