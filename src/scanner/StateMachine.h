#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include <string>

class StateMachine {
public:
	virtual ~StateMachine() {
	}

	virtual void updateState(char inputSymbol) = 0;

	virtual bool isAtFinalState() const = 0;
	virtual std::string getAccumulatedLexeme() const = 0;
	virtual int getAccumulatedLexemeId() const = 0;
};

#endif /* STATEMACHINE_H_ */
