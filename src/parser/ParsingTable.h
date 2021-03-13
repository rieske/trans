#ifndef _PARSING_TABLE_H_
#define _PARSING_TABLE_H_

#include <map>
#include <memory>

#include "LookaheadActionTable.h"
#include "parser/Grammar.h"
#include "scanner/Token.h"

namespace parser {

class LR1Item;
class Action;

class ParsingTable {
public:
	ParsingTable(const Grammar* grammar);
	virtual ~ParsingTable();

	const Action& action(parse_state state, scanner::Token lookahead) const;
	parse_state go_to(parse_state state, int nonterminal) const;
protected:
	const Grammar* grammar;

	std::unordered_map<parse_state, std::unordered_map<int, parse_state>> gotoTable;

	LookaheadActionTable lookaheadActionTable;
};

} // namespace parser

#endif // _PARSING_TABLE_H_
