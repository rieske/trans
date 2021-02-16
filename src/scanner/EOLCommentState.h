#ifndef EOLCOMMENTSTATE_H_
#define EOLCOMMENTSTATE_H_

#include <string>

#include "State.h"

class EOLCommentState: public State {
public:
    EOLCommentState(std::string stateName);
    virtual ~EOLCommentState();

    const State* nextStateForCharacter(char c) const override;
};

#endif // EOLCOMMENTSTATE_H_
