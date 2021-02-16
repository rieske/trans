#include "EOLCommentState.h"

EOLCommentState::EOLCommentState(std::string stateName) :
        State { stateName, "" } {
}

EOLCommentState::~EOLCommentState() {
}

const State* EOLCommentState::nextStateForCharacter(char c) const {
    return (c != '\n') ? this : State::nextStateForCharacter(c);
}

