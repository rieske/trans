#ifndef EOLCOMMENTSTATE_H_
#define EOLCOMMENTSTATE_H_

#include <memory>
#include <string>

#include "State.h"

class EOLCommentState: public State {
public:
	EOLCommentState(std::string stateName);
	virtual ~EOLCommentState();

	const std::shared_ptr<const State> nextStateForCharacter(char c) const;
};

#endif /* EOLCOMMENTSTATE_H_ */
