#include "LookaheadActionTable.h"

#include <stdexcept>
#include <string>

namespace parser {

namespace {
std::shared_ptr<const std::vector<int>> emptyErrorCandidates() {
    static const auto empty = std::make_shared<const std::vector<int>>();
    return empty;
}
} // namespace

void LookaheadActionTable::addAction(parse_state state, int lookahead, Action actionToAdd) {
    if (state + 1 > stateCount_) {
        stateCount_ = state + 1;
    }
    const StateSymbolKey key { state, lookahead };
    const auto existingIt = lookaheadActions.find(key);
    if (existingIt == lookaheadActions.end()) {
        lookaheadActions.emplace(key, std::move(actionToAdd));
    } else if (!existingIt->second.equals(actionToAdd)) {
        throw std::runtime_error { "Lookahead action conflict at state: " + std::to_string(state) + " existing action: "
                + existingIt->second.serialize() + " attempted add of action: " + actionToAdd.serialize() };
    }
}

Action LookaheadActionTable::action(parse_state state, int lookahead) const {
    const auto actionIt = lookaheadActions.find({ state, lookahead });
    if (actionIt != lookaheadActions.end()) {
        return actionIt->second;
    }
    const auto errorIt = errorActionsByState.find(state);
    if (errorIt != errorActionsByState.end()) {
        return errorIt->second;
    }
    // No stored candidates for this state: still synthesize a bare error when grammar is known.
    if (errorGrammar_ != nullptr) {
        return Action::error(0, emptyErrorCandidates(), errorGrammar_);
    }
    throw std::out_of_range {
        "No action for state " + std::to_string(state) + " lookahead " + std::to_string(lookahead)
    };
}

bool LookaheadActionTable::hasCorrectiveAction(parse_state state, int lookahead) const {
    const auto it = lookaheadActions.find({ state, lookahead });
    return it != lookaheadActions.end() && it->second.isCorrective();
}

size_t LookaheadActionTable::size() const {
    return stateCount_;
}

void LookaheadActionTable::reserve(size_t stateCount) {
    lookaheadActions.reserve(stateCount * 4);
    errorActionsByState.reserve(stateCount);
}

void LookaheadActionTable::setStateCount(size_t stateCount) {
    stateCount_ = stateCount;
}

void LookaheadActionTable::setErrorCandidates(
        parse_state state,
        std::shared_ptr<const std::vector<int>> candidates,
        const Grammar* grammar) {
    if (state + 1 > stateCount_) {
        stateCount_ = state + 1;
    }
    errorGrammar_ = grammar;
    // Skip per-state storage when there are no recovery candidates.
    if (!candidates || candidates->empty()) {
        errorActionsByState.erase(state);
        return;
    }
    errorActionsByState[state] = Action::error(0, std::move(candidates), grammar);
}

} // namespace parser
