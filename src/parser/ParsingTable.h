#ifndef _PARSING_TABLE_H_
#define _PARSING_TABLE_H_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "GrammarSymbol.h"

class LR1Item;
class Action;
class Grammar;

using parse_state = size_t;

class ParsingTable {
public:
	ParsingTable(const Grammar* grammar);
	virtual ~ParsingTable();

	Action& action(parse_state state, std::string lookahead) const;
	parse_state go_to(parse_state state, std::string nonterminal) const;
protected:
	std::unique_ptr<const Grammar> grammar;

	std::unordered_map<parse_state, std::map<std::string, std::unique_ptr<Action>>>lookaheadActions;
	std::unordered_map<parse_state, std::map<std::string, parse_state>> gotoTable;
};

#endif // _PARSING_TABLE_H_
