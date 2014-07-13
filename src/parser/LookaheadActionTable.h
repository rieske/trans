#ifndef LOOKAHEADACTIONTABLE_H_
#define LOOKAHEADACTIONTABLE_H_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace parser {

class Action;

using parse_state = size_t;

class LookaheadActionTable {
public:
	LookaheadActionTable();
	virtual ~LookaheadActionTable();

	void addAction(parse_state state, std::string lookahead, std::unique_ptr<Action> actionToAdd);
	const Action& action(parse_state state, std::string lookahead) const;
	size_t size() const;

private:
	std::unordered_map<parse_state, std::map<std::string, std::unique_ptr<Action>>>lookaheadActions;
};

}

#endif /* LOOKAHEADACTIONTABLE_H_ */
