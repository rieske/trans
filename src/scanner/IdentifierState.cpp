#include "IdentifierState.h"

using std::string;

IdentifierState::IdentifierState(string stateName, int tokenId) :
		State { stateName, tokenId } {
}

IdentifierState::~IdentifierState() {
}

bool IdentifierState::needsKeywordLookup() const {
	return true;
}

