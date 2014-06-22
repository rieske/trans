#include "EOLCommentState.h"

using std::string;

EOLCommentState::EOLCommentState(string stateName) :
		State { stateName, "" } {
}

EOLCommentState::~EOLCommentState() {
}

const State* EOLCommentState::nextStateForCharacter(char c) const {
	return (c != '\n') ? this : State::nextStateForCharacter(c);
}

