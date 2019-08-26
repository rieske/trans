#include "LookaheadActionTable.h"

#include <algorithm>
#include <iostream>

#include "Action.h"

using std::string;

namespace parser {

LookaheadActionTable::LookaheadActionTable() {
}

LookaheadActionTable::~LookaheadActionTable() {
}

void LookaheadActionTable::addAction(parse_state state, string lookahead, std::unique_ptr<Action> actionToAdd) {
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

const Action& LookaheadActionTable::action(parse_state state, string lookahead) const {
    return *lookaheadActions.at(state).at(lookahead);
}

size_t LookaheadActionTable::size() const {
    return lookaheadActions.size();
}

}
