#ifndef FINITEAUTOMATONFACTORY_H_
#define FINITEAUTOMATONFACTORY_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "FiniteAutomaton.h"

class State;

class LexFileFiniteAutomaton: public FiniteAutomaton {
public:
	LexFileFiniteAutomaton(std::string configurationFileName);
	virtual ~LexFileFiniteAutomaton();

private:
	int nextKeywordId { 1 };
	std::map<std::string, std::unique_ptr<State>> namedStates;

	State* addNewState(std::string stateDefinitionRecord);
	std::pair<std::string, std::string> createNamedTransitionPair(std::string transitionDefinitionRecord);
	void parseKeywords(std::string keywordsRecord);

	friend std::ostream& operator<<(std::ostream& os, const LexFileFiniteAutomaton& factory);
};

#endif /* FINITEAUTOMATONFACTORY_H_ */
