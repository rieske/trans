#include "State.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>

#include "CommentState.h"
#include "EOLCommentState.h"
#include "StringLiteralState.h"

using std::shared_ptr;
using std::string;
using std::vector;

static const char IDENTIFIER = '%';
static const char STRING_LITERAL = '"';
static const char COMMENT = '!';
static const char EOL_COMMENT = '/';

shared_ptr<State> State::createState(std::string stateDefinitionRecord) {
	if (stateDefinitionRecord.empty()) {
		throw std::invalid_argument("Empty state record");
	}
	std::istringstream stateDefinitionStream { stateDefinitionRecord };
	string stateDefinition;
	stateDefinitionStream >> stateDefinition;
	int tokenId;
	stateDefinitionStream >> tokenId;
	string stateName = stateDefinition.substr(1, stateDefinition.length());
	char stateType = stateDefinition.at(0);
	switch (stateType) {
	case IDENTIFIER:
		return shared_ptr<State> { new State(stateName, tokenId, stateType) };
	case COMMENT:
		return shared_ptr<State> { new CommentState(stateName) };
	case EOL_COMMENT:
		return shared_ptr<State> { new EOLCommentState(stateName) };
	case STRING_LITERAL:
		return shared_ptr<State> { new StringLiteralState(stateName, tokenId) };
	default:
		return shared_ptr<State> { new State(stateDefinition, tokenId, stateType) };
	}
}

State::State(string stateName, int tokenId, char stateType) :
		stateName { stateName },
		tokenId { tokenId },
		wildcardTransition { nullptr } {
	switch (stateType) {
	case IDENTIFIER:
		identifier = true;
		break;
	default:
		;
	}
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
	if (transitions.find(c) != transitions.end()) {
		return transitions.at(c);
	} else if (wildcardTransition != nullptr) {
		return wildcardTransition;
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
	if (stringLiteral && (c == ' '))
		return "CURRENT";
	if (stringLiteral && (c == '\n'))
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

bool State::isIdentifier() const {
	return identifier;
}

bool State::isComment() const {
	return comment;
}

void State::setKeywordCheck() {
	this->identifier = true;
}

void State::setIgnoreSpaces() {
	this->stringLiteral = true;
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
