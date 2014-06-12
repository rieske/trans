#include "IdentifierState.h"

using std::string;

IdentifierState::IdentifierState(string stateName, string tokenId) :
		State { stateName, tokenId } {
}

IdentifierState::~IdentifierState() {
}

bool IdentifierState::needsKeywordLookup() const {
	return true;
}

