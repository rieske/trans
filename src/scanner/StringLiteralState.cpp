#include "StringLiteralState.h"

#include <iostream>
#include <stdexcept>

using std::string;

StringLiteralState::StringLiteralState(std::string stateName, int tokenId) :
		State { stateName, tokenId, '\0' } {
}

StringLiteralState::~StringLiteralState() {
}

const std::shared_ptr<const State> StringLiteralState::nextStateForCharacter(char c) const {
	if (c == ' ') {
		return shared_from_this();
	}
	if (c == '\n') {
		std::cerr << "error in string literal state";
		throw std::invalid_argument("error in string literal state");
	}
	return State::nextStateForCharacter(c);
}

