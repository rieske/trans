#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

class Token;

class StateMachine {
public:
	virtual ~StateMachine() {
	}

	virtual void updateState(char inputSymbol) = 0;

	virtual bool isAtFinalState() const = 0;
	virtual Token getCurrentToken() = 0;
};

#endif /* STATEMACHINE_H_ */
