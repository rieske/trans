#ifndef EOLCOMMENTSTATE_H_
#define EOLCOMMENTSTATE_H_

#include <string>

#include "State.h"

namespace scanner {

class EOLCommentState: public State {
public:
    EOLCommentState(std::string stateName);
    virtual ~EOLCommentState();

    const State* nextStateForCharacter(char c) const override;
};

} // namespace scanner

#endif // EOLCOMMENTSTATE_H_
