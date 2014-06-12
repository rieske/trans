#include "EOLCommentState.h"

using std::string;
using std::shared_ptr;

EOLCommentState::EOLCommentState(string stateName) :
		State { stateName, "" } {
}

EOLCommentState::~EOLCommentState() {
}

const std::shared_ptr<const State> EOLCommentState::nextStateForCharacter(char c) const {
	return (c != '\n') ? shared_from_this() : State::nextStateForCharacter(c);
}

