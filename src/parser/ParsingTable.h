#ifndef _PARSING_TABLE_H_
#define _PARSING_TABLE_H_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "Grammar.h"

class Action;
class GrammarSymbol;

using parse_state = size_t;

class ParsingTable {
public:
	virtual ~ParsingTable();

	Action& action(parse_state state, std::string terminal) const;
	parse_state go_to(parse_state state, const GrammarSymbol* nonterminal) const;

protected:
	std::unordered_map<parse_state, std::map<std::string, std::unique_ptr<Action>>> terminalActionTables;
	std::unordered_map<parse_state, std::map<const GrammarSymbol*, parse_state>> gotoTable;
};

#endif // _PARSING_TABLE_H_
