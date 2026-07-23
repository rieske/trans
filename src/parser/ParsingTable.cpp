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
    // Use TokenStream's classification as-is. Do not re-promote id -> typedef_name:
    // TokenStream demotes typedef spellings to id after `*` / `.` / `->` so a member
    // can reuse a typedef name (git: pcre2_match_data *pcre2_match_data). Re-promoting
    // would look up the wrong action and fail the parse. Fresh typedefs are already
    // reclassified on each getCurrentToken() via TypedefRegistry.
    int lookaheadId = grammar->symbolId(lookahead.id);
    return lookaheadActionTable.action(state, lookaheadId);
}

parse_state ParsingTable::go_to(parse_state state, int nonterminal) const {
    return gotoTable.at({ state, nonterminal });
}

} // namespace parser
