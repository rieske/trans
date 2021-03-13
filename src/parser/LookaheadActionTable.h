#ifndef LOOKAHEADACTIONTABLE_H_
#define LOOKAHEADACTIONTABLE_H_

#include <memory>
#include <unordered_map>

namespace parser {

class Action;

using parse_state = size_t;

class LookaheadActionTable {
public:
	LookaheadActionTable();
	virtual ~LookaheadActionTable();

	void addAction(parse_state state, int lookahead, std::unique_ptr<Action> actionToAdd);
	const Action& action(parse_state state, int lookahead) const;
	size_t size() const;

private:
	std::unordered_map<parse_state, std::unordered_map<int, std::unique_ptr<Action>>>lookaheadActions;
};

} // namespace parser

#endif // LOOKAHEADACTIONTABLE_H_
