#include "IdentifierState.h"

IdentifierState::IdentifierState(std::string stateName, std::string tokenId) :
        State { stateName, tokenId } {
}

IdentifierState::~IdentifierState() {
}

bool IdentifierState::needsKeywordLookup() const {
    return true;
}

