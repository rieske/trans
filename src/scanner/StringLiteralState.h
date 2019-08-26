#ifndef STRINGLITERALSTATE_H_
#define STRINGLITERALSTATE_H_

#include <memory>
#include <string>

#include "State.h"

class StringLiteralState: public State {
public:
	StringLiteralState(std::string stateName, std::string tokenId);
	virtual ~StringLiteralState();

	const State* nextStateForCharacter(char c) const override;
};

#endif /* STRINGLITERALSTATE_H_ */
