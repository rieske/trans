#include <stdio.h>
#include "State.h"

State::State() {
	id = 0;
	lookup = false;
	ignoreSpaces = false;
	comment = false;
	eol_comment = false;
}

State::~State() {
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

string State::get_next(char c) const {
	if (eol_comment)
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

int State::getId() const {
	return id;
}

bool State::need_lookup() const {
	return lookup;
}

bool State::is_comment() const {
	return comment;
}

bool State::is_eol_comment() const {
	return eol_comment;
}

void State::setId(string id) {
	this->id = atoi(id.c_str());
}

void State::setKeywordCheck() {
	this->lookup = true;
}

void State::setIgnoreSpaces() {
	this->ignoreSpaces = true;
}

void State::setComment() {
	this->comment = true;
}

void State::setEolComment() {
	this->eol_comment = true;
}
