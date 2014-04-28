#ifndef _STATE_H_
#define _STATE_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>

class State: public std::enable_shared_from_this<State> {
public:
	State(std::string stateName, int tokenId);
	virtual ~State();

	void addTransition(std::string charactersForTransition, std::shared_ptr<State> state);
	virtual const std::shared_ptr<const State> nextStateForCharacter(char c) const;
	std::string getName() const;

	void outputState(std::ostream& ostream) const;

	int getTokenId() const;
	virtual bool needsKeywordLookup() const;
	bool isFinal() const;

private:
	std::string stateName;
	int tokenId { 0 };

	std::shared_ptr<State> wildcardTransition;
	std::map<char, std::shared_ptr<State>> transitions;
};

#endif // _STATE_H_
