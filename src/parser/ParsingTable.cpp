#include "ParsingTable.h"

#include <cstdlib>
#include <iostream>

using std::string;

ParsingTable::ParsingTable() {
}

ParsingTable::~ParsingTable() {
}

Action& ParsingTable::action(parse_state state, string terminal) const {
	return *terminalActionTables[state].at(terminal);
}

parse_state ParsingTable::go_to(parse_state state, std::shared_ptr<const GrammarSymbol> nonterminal) const {
	return gotoTable.at(state).at(nonterminal);
}
