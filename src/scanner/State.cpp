#include "State.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "EOLCommentState.h"
#include "IdentifierState.h"
#include "StringLiteralState.h"

using std::shared_ptr;
using std::string;
using std::vector;

State::State(string stateName, int tokenId) :
		stateName { stateName },
		tokenId { tokenId },
		wildcardTransition { nullptr } {
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
	}
	if (wildcardTransition != nullptr) {
		return wildcardTransition;
	}
	throw std::invalid_argument { "Can't reach next state for given input: " + string { c } };
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

bool State::needsKeywordLookup() const {
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
