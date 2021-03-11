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
    try {
        std::string lookaheadId = std::to_string(grammar->symbolId(lookahead.id));
        return lookaheadActionTable.action(state, lookaheadId);
    } catch (std::out_of_range& e) {
        std::cerr << "caught!!!!!!\n";
        std::cerr << "state: " << state << " lookahead: " << lookahead.lexeme << " lookahead id: " << lookahead.id << std::endl;
        throw e;
    }
}

parse_state ParsingTable::go_to(parse_state state, std::string nonterminal) const {
    return gotoTable.at(state).at(nonterminal);
}

} // namespace parser

