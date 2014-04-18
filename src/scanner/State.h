#ifndef _STATE_H_
#define _STATE_H_

#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <memory>

class State : public std::enable_shared_from_this<State> {
public:
	State(std::string stateDefinitionRecord, int stateId);

	State(); // deprecated
	~State();

	void addTransition(std::string charactersForTransition, std::shared_ptr<State> state);
	const std::shared_ptr<const State> nextStateForCharacter(char c) const;

	std::string getName() const;

	// deprecated
	int add_state(std::string next_state, std::string name);

	void listing(FILE *) const;

	std::string nextStateNameForCharacter(char c) const;

	int getTokenId() const;
	bool isPossibleKeyword() const;
	bool isComment() const;

	void setKeywordCheck();
	void setIgnoreSpaces();
	void setComment();
	void setEolComment();
	void setTokenId(std::string id);
private:
	std::vector<std::string> v_state;             // vardai būsenų, į kurias galima patekti
	std::vector<std::string> v_chars;             // simboliai, keičiantys būseną pagal indeksą sąraše

	std::string stateName;
	int stateId;
	int tokenId { 0 };
	std::shared_ptr<State> wildcardTransition;
	std::map<char, std::shared_ptr<State>> transitions;

	bool possibleKeyword { false };
	bool ignoreSpaces { false };
	bool comment { false };
	bool eolComment { false };
};

#endif // _STATE_H_
