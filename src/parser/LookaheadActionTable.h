#ifndef LOOKAHEADACTIONTABLE_H_
#define LOOKAHEADACTIONTABLE_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "Action.h"
#include "HashCombine.h"

namespace parser {

class LookaheadActionTable {
public:
	struct ExplicitAction {
		parse_state state;
		int lookahead;
		Action action;
	};
	struct ErrorRow {
		parse_state state;
		std::vector<int> candidates;
	};

	void addAction(parse_state state, int lookahead, Action actionToAdd);
	const Action* findAction(parse_state state, int lookahead) const;
	Action action(parse_state state, int lookahead) const;
	bool hasCorrectiveAction(parse_state state, int lookahead) const;
	size_t size() const;
	void reserve(size_t stateCount);
	void setStateCount(size_t stateCount);

	void setErrorCandidates(parse_state state, std::vector<int> candidates);
	std::shared_ptr<const std::vector<int>> errorCandidates(parse_state state) const;

	std::vector<ExplicitAction> explicitActions() const;
	std::vector<ErrorRow> errorRows() const;

private:
	std::unordered_map<StateSymbolKey, Action, StateSymbolHash> lookaheadActions;
	std::unordered_map<parse_state, std::shared_ptr<const std::vector<int>>> errorCandidatesByState;
	size_t stateCount_ { 0 };
};

} // namespace parser

#endif
