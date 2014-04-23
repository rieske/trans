#ifndef IDENTIFIERSTATE_H_
#define IDENTIFIERSTATE_H_

#include <string>

#include "State.h"

class IdentifierState: public State {
public:
	IdentifierState(std::string stateName, int tokenId);
	virtual ~IdentifierState();

	bool needsKeywordLookup() const;
};

#endif /* IDENTIFIERSTATE_H_ */
