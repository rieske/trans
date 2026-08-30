#ifndef PARSER_GENERATE_PARSING_TABLE_H_
#define PARSER_GENERATE_PARSING_TABLE_H_

#include "parser/CanonicalCollection.h"
#include "parser/ParsingTable.h"

namespace parser {

ParsingTable generateParsingTable(const Grammar* grammar,
        AutomatonKind kind = AutomatonKind::LALR1);

} // namespace parser

#endif
