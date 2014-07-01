#include "ParsingTable.h"

#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "Action.h"
#include "Grammar.h"
#include "LR1Item.h"

using std::string;

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

ParsingTable::ParsingTable(const Grammar* grammar) :
		grammar { grammar } {
	logger << *this->grammar;
}

ParsingTable::~ParsingTable() {
}

const Action& ParsingTable::action(parse_state state, string lookahead) const {
	return lookaheadActionTable.action(state, lookahead);
}

parse_state ParsingTable::go_to(parse_state state, string nonterminal) const {
	return gotoTable.at(state).at(nonterminal);
}
