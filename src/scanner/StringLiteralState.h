#ifndef STRINGLITERALSTATE_H_
#define STRINGLITERALSTATE_H_

#include <string>

#include "State.h"

namespace scanner {

class StringLiteralState: public State {
public:
    StringLiteralState(std::string stateName, std::string tokenId);
    virtual ~StringLiteralState();

    const State* nextStateForCharacter(char c) const override;
};

} // namespace scanner

#endif // STRINGLITERALSTATE_H_
