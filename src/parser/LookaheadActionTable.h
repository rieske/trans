#ifndef LOOKAHEADACTIONTABLE_H_
#define LOOKAHEADACTIONTABLE_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "Action.h"
#include "HashCombine.h"

namespace parser {

class Grammar;

class LookaheadActionTable {
public:
	LookaheadActionTable() = default;

	void addAction(parse_state state, int lookahead, Action actionToAdd);
	Action action(parse_state state, int lookahead) const;
	// Single lookup: explicit action present and isCorrective() (used for error recovery).
	bool hasCorrectiveAction(parse_state state, int lookahead) const;
	size_t size() const;
	void reserve(size_t stateCount);
	void setStateCount(size_t stateCount);

	// Register per-state error recovery candidates for missing terminals.
	// Empty candidate lists are not stored; missing cells synthesize Error with no hints.
	void setErrorCandidates(parse_state state, std::shared_ptr<const std::vector<int>> candidates, const Grammar* grammar);

private:
	std::unordered_map<StateSymbolKey, Action, StateSymbolHash> lookaheadActions;
	std::unordered_map<parse_state, Action> errorActionsByState;
	const Grammar* errorGrammar_ { nullptr };
	size_t stateCount_ { 0 };
};

} // namespace parser

#endif // LOOKAHEADACTIONTABLE_H_
