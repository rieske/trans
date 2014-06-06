#ifndef FINITEAUTOMATON_H_
#define FINITEAUTOMATON_H_

#include <map>
#include <memory>
#include <string>

#include "StateMachine.h"

class State;

class FiniteAutomaton: public StateMachine {
public:
	FiniteAutomaton(std::shared_ptr<State> startState, std::map<std::string, unsigned> keywordIds);
	virtual ~FiniteAutomaton();

	void updateState(char inputSymbol);

	bool isAtFinalState() const;
	std::string getAccumulatedLexeme() const;
	int getAccumulatedLexemeId() const;

private:
	std::string accumulator;
	std::string accumulatedLexeme;
	int accumulatedTokenId { 0 };

	std::shared_ptr<const State> startState;
	std::shared_ptr<const State> currentState;
	std::map<std::string, unsigned> keywordIds;
};

#endif /* FINITEAUTOMATON_H_ */
