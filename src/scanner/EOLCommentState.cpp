#include "EOLCommentState.h"

namespace scanner {

EOLCommentState::EOLCommentState(std::string stateName) :
        State { stateName, "" } {
}

EOLCommentState::~EOLCommentState() {
}

const State* EOLCommentState::nextStateForCharacter(char c) const {
    return (c != '\n') ? this : State::nextStateForCharacter(c);
}

} // namespace scanner

