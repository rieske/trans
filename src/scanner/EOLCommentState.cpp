#include "EOLCommentState.h"

using std::string;
using std::shared_ptr;

EOLCommentState::EOLCommentState(string stateName, shared_ptr<State> startState) :
		State { stateName, 0 },
		startState { startState } {
}

EOLCommentState::~EOLCommentState() {
}

const std::shared_ptr<const State> EOLCommentState::nextStateForCharacter(char c) const {
	return (c == '\n') ? startState : shared_from_this();
}
