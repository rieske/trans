#include "ParsingTable.h"

#include "util/Logger.h"
#include "util/LogManager.h"

namespace {
static Logger& logger = LogManager::getComponentLogger(Component::PARSER);
} // namespace

namespace parser {

ParsingTable::ParsingTable(const Grammar* grammar) :
		grammar { grammar } {
	logger << *this->grammar;
}

ParsingTable::~ParsingTable() {
}

const Action& ParsingTable::action(parse_state state, scanner::Token lookahead) const {
    std::string lookaheadId = std::to_string(grammar->symbolId(lookahead.id));
    return lookaheadActionTable.action(state, lookaheadId);
}

parse_state ParsingTable::go_to(parse_state state, std::string nonterminal) const {
    return gotoTable.at(state).at(nonterminal);
}

} // namespace parser

