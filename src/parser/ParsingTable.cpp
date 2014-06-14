#include "ParsingTable.h"

#include "Action.h"

using std::string;

ParsingTable::~ParsingTable() {
	delete[] terminalActionTables;
}

Action& ParsingTable::action(parse_state state, string terminal) const {
	return *terminalActionTables[state].at(terminal);
}

parse_state ParsingTable::go_to(parse_state state, std::shared_ptr<const GrammarSymbol> nonterminal) const {
	return gotoTable.at(state).at(nonterminal);
}
