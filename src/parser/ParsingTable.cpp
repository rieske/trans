#include "ParsingTable.h"

#include "Action.h"
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

Action ParsingTable::action(parse_state state, scanner::Token lookahead) const {
    if (const auto lookaheadId = grammar->trySymbolId(lookahead.id)) {
        return lookaheadActionTable.action(state, *lookaheadId);
    }
    return Action::error(state, LookaheadActionTable::emptyErrorCandidates(), grammar);
}

parse_state ParsingTable::go_to(parse_state state, int nonterminal) const {
    return gotoTable.at({ state, nonterminal });
}

std::optional<parse_state> ParsingTable::tryGoTo(parse_state state, int nonterminal) const {
    auto found = gotoTable.find({ state, nonterminal });
    if (found == gotoTable.end()) {
        return std::nullopt;
    }
    return found->second;
}

} // namespace parser

