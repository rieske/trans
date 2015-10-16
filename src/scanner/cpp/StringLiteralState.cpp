#include "scanner/StringLiteralState.h"

#include <stdexcept>

using std::string;

StringLiteralState::StringLiteralState(string stateName, string tokenId) :
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

