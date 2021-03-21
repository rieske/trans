#include "LookaheadActionTable.h"

#include "Action.h"

namespace parser {

LookaheadActionTable::LookaheadActionTable() {
}

LookaheadActionTable::~LookaheadActionTable() {
}

void LookaheadActionTable::addAction(parse_state state, int lookahead, std::unique_ptr<Action> actionToAdd) {
    if (lookaheadActions[state].find(lookahead) == lookaheadActions[state].end()) {
        lookaheadActions[state][lookahead] = std::move(actionToAdd);
    } else {
        auto& existingAction = lookaheadActions[state].at(lookahead);
        if (existingAction->serialize() != actionToAdd->serialize()) {
            throw std::runtime_error { "Lookahead action conflict at state: " + std::to_string(state) + " existing action: "
                    + existingAction->serialize() + " attempted add of action: " + actionToAdd->serialize() };
        }
    }
}

const Action& LookaheadActionTable::action(parse_state state, int lookahead) const {
    return *lookaheadActions.at(state).at(lookahead);
}

bool LookaheadActionTable::hasAction(parse_state state, int lookahead) const {
    const auto& stateActions = lookaheadActions.at(state);
    return stateActions.find(lookahead) != stateActions.end();
}

size_t LookaheadActionTable::size() const {
    return lookaheadActions.size();
}

} // namespace parser

