#ifndef FINITEAUTOMATON_H_
#define FINITEAUTOMATON_H_

#include <map>
#include <memory>
#include <string>

#include "StateMachine.h"

class State;

class FiniteAutomaton: public StateMachine {
public:
	FiniteAutomaton(State* startState, std::map<std::string, unsigned> keywordIds);
	virtual ~FiniteAutomaton();

	void updateState(char inputSymbol) override;

	bool isAtFinalState() const override;
	std::string getAccumulatedLexeme() const override;
	std::string getAccumulatedToken() const override;

protected:
	FiniteAutomaton() {
	}

	const State* startState { nullptr };
	const State* currentState { nullptr };
	std::map<std::string, unsigned> keywordIds;
private:

	std::string accumulator;
	std::string accumulatedLexeme;
	std::string accumulatedToken;
};

#endif /* FINITEAUTOMATON_H_ */
