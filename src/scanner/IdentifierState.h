#ifndef IDENTIFIERSTATE_H_
#define IDENTIFIERSTATE_H_

#include <memory>
#include <string>

#include "State.h"

namespace scanner {

class IdentifierState: public State {
public:
    IdentifierState(std::string stateName, std::string tokenId);
    virtual ~IdentifierState();

    bool needsKeywordLookup() const;
};

} // namespace scanner

#endif // IDENTIFIERSTATE_H_
