#ifndef PARSER_COMPILE_PARSING_TABLE_H_
#define PARSER_COMPILE_PARSING_TABLE_H_

#include <string>
#include <unordered_map>

#include "HashCombine.h"
#include "LookaheadActionTable.h"
#include "ParsingTable.h"

namespace parser {

ParsingTable compileParsingTable(
        const LookaheadActionTable& actions,
        const std::unordered_map<StateSymbolKey, parse_state, StateSymbolHash>& gotos,
        const Grammar& grammar);

void writeParsingTableSource(const ParsingTable& table, const std::string& path);

} // namespace parser

#endif
