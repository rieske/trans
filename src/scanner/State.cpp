#include "State.h"

#include <cctype>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <iostream>

using std::string;
using std::vector;

const char KEYWORD = '%';
const char IGNORE_SPACES = '"';
const char COMMENT = '!';
const char EOL_COMMENT = '/';

State::State(string stateDefinitionRecord, int stateId) :
		stateId { stateId },
		wildcardTransition { nullptr } {
	if (stateDefinitionRecord.empty()) {
		throw std::invalid_argument("Empty state record");
	}
	std::istringstream stateDefinitionStream { stateDefinitionRecord };
	string stateDefinition;
	stateDefinitionStream >> stateDefinition;
	stateName = stateDefinition.substr(1, stateDefinition.length());
	char stateType = stateDefinition.at(0);
	switch (stateType) {
	case KEYWORD:
		possibleKeyword = true;
		break;
	case COMMENT:
		comment = true;
		break;
	case EOL_COMMENT:
		eolComment = true;
		break;
	case IGNORE_SPACES:
		ignoreSpaces = true;
		break;
	default:
		stateName = stateDefinition;
		break;
	}
	stateDefinitionStream >> tokenId;
}

State::State() {
}

State::~State() {
}

string State::getName() const {
	return stateName;
}

void State::addTransition(std::string charactersForTransition, std::shared_ptr<State> state) {
	if (charactersForTransition.empty()) {
		wildcardTransition = state;
	} else {
		for (char characterForTransition : charactersForTransition) {
			transitions[characterForTransition] = state;
		}
	}
}

const std::shared_ptr<const State> State::nextStateForCharacter(char c) const {
	if (eolComment) {
		return (c == '\n') ? nullptr : shared_from_this();
	}
	if (ignoreSpaces && (c == ' ')) {
		return shared_from_this();
	}
	if (ignoreSpaces && (c == '\n')) {
		std::cerr << "error state";
		throw std::invalid_argument("error state");
	}
	if (transitions.find(c) != transitions.end()) {
		return transitions.at(c);
	} else if (wildcardTransition != nullptr) {
		return wildcardTransition;
	}
	if (comment) {
		return shared_from_this();
	}
	if (isspace(c) || c == 0) {
		return nullptr;
	}
	std::cerr << "error state";
	throw std::invalid_argument("error state");
}

int State::add_state(string next, string ch) {
	v_state.push_back(next);
	v_chars.push_back(ch);
	return 0;
}

void State::listing(FILE *logfile) const {
	for (unsigned int i = 0; i < v_state.size(); i++)
		fprintf(logfile, "\t%s:\t%s\n", v_state[i].c_str(), v_chars[i].c_str());
}

string State::nextStateNameForCharacter(char c) const {
	if (eolComment)
		return "EOL_COMMENT";
	if (ignoreSpaces && (c == ' '))
		return "CURRENT";
	if (ignoreSpaces && (c == '\n'))
		return "ERROR";

	unsigned int i, j;
	for (i = 0; i < v_chars.size(); i++) {
		if (v_chars[i].empty())
			return v_state[i];
		for (j = 0; j < v_chars[i].size(); j++)
			if (v_chars[i][j] == c)
				return v_state[i];
	}

	if (comment)
		return "CURRENT";
	if (isspace(c) || c == 0)
		return "NONE";
	return "ERROR";
}

int State::getTokenId() const {
	return tokenId;
}

bool State::isPossibleKeyword() const {
	return possibleKeyword;
}

bool State::isComment() const {
	return comment;
}

void State::setKeywordCheck() {
	this->possibleKeyword = true;
}

void State::setIgnoreSpaces() {
	this->ignoreSpaces = true;
}

void State::setComment() {
	this->comment = true;
}

void State::setEolComment() {
	this->eolComment = true;
}

void State::setTokenId(string id) {
	this->tokenId = atoi(id.c_str());
}
