#ifndef _PARSING_TABLE_H_
#define _PARSING_TABLE_H_

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>

#include "LookaheadActionTable.h"
#include "parser/Grammar.h"
#include "parser/HashCombine.h"
#include "scanner/Token.h"

namespace parser {

class Action;

class ParsingTable {
public:
	ParsingTable(const Grammar* grammar);
	virtual ~ParsingTable() = default;

	Action action(parse_state state, const scanner::Token& lookahead) const;
	parse_state go_to(parse_state state, int nonterminal) const;
	std::optional<parse_state> tryGoTo(parse_state state, int nonterminal) const;
	const Grammar* getGrammar() const { return grammar; }
	std::size_t stateCount() const;
	void persistToFile(const std::string& fileName) const;

protected:
	void loadFromFile(const std::string& fileName);

	const Grammar* grammar;

	std::unordered_map<StateSymbolKey, parse_state, StateSymbolHash> gotoTable;

	LookaheadActionTable lookaheadActionTable;
};

} // namespace parser

#endif // _PARSING_TABLE_H_
