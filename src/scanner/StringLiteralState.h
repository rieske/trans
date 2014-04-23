#ifndef STRINGLITERALSTATE_H_
#define STRINGLITERALSTATE_H_

#include <memory>
#include <string>

#include "State.h"

class StringLiteralState: public State {
public:
	StringLiteralState(std::string stateName, int tokenId);
	virtual ~StringLiteralState();

	const std::shared_ptr<const State> nextStateForCharacter(char c) const;
};

#endif /* STRINGLITERALSTATE_H_ */
