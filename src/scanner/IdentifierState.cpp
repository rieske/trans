#include "IdentifierState.h"

namespace scanner {

IdentifierState::IdentifierState(std::string stateName, std::string tokenId) :
        State { stateName, tokenId } {
}

IdentifierState::~IdentifierState() {
}

bool IdentifierState::needsKeywordLookup() const {
    return true;
}

} // namespace scanner

