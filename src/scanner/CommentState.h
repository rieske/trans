#ifndef COMMENTSTATE_H_
#define COMMENTSTATE_H_

#include <memory>
#include <string>

#include "State.h"

class CommentState: public State {
public:
	CommentState(std::string stateName);
	virtual ~CommentState();

	const std::shared_ptr<const State> nextStateForCharacter(char c) const;
};

#endif /* COMMENTSTATE_H_ */
