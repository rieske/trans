#ifndef _STATE_H_
#define _STATE_H_

#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

class State: public std::enable_shared_from_this<State> {
public:
	static std::shared_ptr<State> createState(std::string stateDefinitionRecord);
	virtual ~State();

	State(); // deprecated

	void addTransition(std::string charactersForTransition, std::shared_ptr<State> state);
	virtual const std::shared_ptr<const State> nextStateForCharacter(char c) const;

	std::string getName() const;

	// deprecated
	int add_state(std::string next_state, std::string name);

	void listing(FILE *) const;

	std::string nextStateNameForCharacter(char c) const;

	int getTokenId() const;
	virtual bool needsKeywordLookup() const;

	bool isComment() const;

	void setKeywordCheck();
	void setIgnoreSpaces();
	void setComment();
	void setEolComment();
	void setTokenId(std::string id);

protected:
	State(std::string stateName, int tokenId);

private:
	std::vector<std::string> v_state;             // vardai būsenų, į kurias galima patekti
	std::vector<std::string> v_chars;             // simboliai, keičiantys būseną pagal indeksą sąraše

	std::string stateName;
	int tokenId { 0 };

	std::shared_ptr<State> wildcardTransition;
	std::map<char, std::shared_ptr<State>> transitions;

	bool identifier { false };
	bool stringLiteral { false };
	bool comment { false };
	bool eolComment { false };
};

#endif // _STATE_H_
