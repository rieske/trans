#include "parser/ParsingTable.h"

#include "scanner/Token.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "parser/Grammar.h"

using std::string;

namespace parser {

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

ParsingTable::ParsingTable(const Grammar* grammar) :
		grammar { grammar } {
	logger << *this->grammar;
}

ParsingTable::~ParsingTable() {
}

const Action& ParsingTable::action(parse_state state, Token lookahead) const {
	return lookaheadActionTable.action(state, lookahead.id);
}

parse_state ParsingTable::go_to(parse_state state, string nonterminal) const {
	return gotoTable.at(state).at(nonterminal);
}

}
