#include "ParsingTable.h"

#include "Action.h"

using std::string;

ParsingTable::~ParsingTable() {
}

Action& ParsingTable::action(parse_state state, string terminal) const {
	return *terminalActionTables.at(state).at(terminal);
}

parse_state ParsingTable::go_to(parse_state state, const GrammarSymbol* nonterminal) const {
	return gotoTable.at(state).at(nonterminal);
}
