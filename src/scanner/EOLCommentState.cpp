#include "EOLCommentState.h"

using std::string;

EOLCommentState::EOLCommentState(string stateName) :
		State { stateName, -1 } {
}

EOLCommentState::~EOLCommentState() {
}

const std::shared_ptr<const State> EOLCommentState::nextStateForCharacter(char c) const {
	return (c == '\n') ? nullptr : shared_from_this();
}
