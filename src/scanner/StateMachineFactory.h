#ifndef STATEMACHINEFACTORY_H_
#define STATEMACHINEFACTORY_H_

#include <memory>

class StateMachine;

class StateMachineFactory {
public:
	virtual ~StateMachineFactory() {
	}

	virtual std::unique_ptr<StateMachine> createAutomaton() const = 0;
};

#endif /* STATEMACHINEFACTORY_H_ */
