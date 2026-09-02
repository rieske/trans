#ifndef _PARSING_TABLE_H_
#define _PARSING_TABLE_H_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Action.h"
#include "parser/Grammar.h"
#include "scanner/Token.h"

namespace parser {

struct ParsingTableAccess;

class ParsingTable {
public:
	static constexpr uint8_t kCellEmpty = 0;
	static constexpr uint8_t kCellShift = 1;
	static constexpr uint8_t kCellReduce = 2;
	static constexpr uint8_t kCellAccept = 3;

	struct ActionCell {
		uint8_t kind { kCellEmpty };
		uint16_t payload { 0 };
	};

	explicit ParsingTable(const Grammar* grammar);

	Action action(parse_state state, const scanner::Token& lookahead) const;
	ActionCell cell(parse_state state, int symbolId) const;
	parse_state go_to(parse_state state, int nonterminal) const;
	std::optional<parse_state> tryGoTo(parse_state state, int nonterminal) const;
	const Grammar* getGrammar() const { return grammar_; }
	std::size_t stateCount() const { return stateCount_; }

private:
	friend struct ParsingTableAccess;

	ParsingTable() = default;
	void validate() const;
	void loadProduct();

	const Grammar* grammar_ { nullptr };
	std::size_t stateCount_ { 0 };
	std::size_t ruleCount_ { 0 };
	int minTerminal_ { 0 };
	int maxTerminal_ { 0 };
	int terminalColumns_ { 0 };
	int minNonterminal_ { 0 };
	int maxNonterminal_ { 0 };
	int nonterminalColumns_ { 0 };
	std::vector<uint8_t> actionKind_;
	std::vector<uint16_t> actionPayload_;
	std::vector<int16_t> gotos_;
	std::vector<uint32_t> errorOffset_;
	std::vector<int> errorCandidates_;
	std::vector<int> terminalIds_;
};

} // namespace parser

#endif // _PARSING_TABLE_H_
