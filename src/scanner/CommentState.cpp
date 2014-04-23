#include "CommentState.h"

using std::string;

CommentState::CommentState(string stateName) :
		State { stateName, -1 } {
}

CommentState::~CommentState() {
}

const std::shared_ptr<const State> CommentState::nextStateForCharacter(char c) const {
	return nullptr;
}
