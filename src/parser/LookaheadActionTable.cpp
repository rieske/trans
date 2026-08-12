#include "LookaheadActionTable.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace parser {

void LookaheadActionTable::addAction(parse_state state, int lookahead, Action actionToAdd) {
    if (actionToAdd.kind() == Action::Kind::Error) {
        throw std::runtime_error { "error actions are not explicit cells" };
    }
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

const Action* LookaheadActionTable::findAction(parse_state state, int lookahead) const {
    const auto actionIt = lookaheadActions.find({ state, lookahead });
    if (actionIt == lookaheadActions.end()) {
        return nullptr;
    }
    return &actionIt->second;
}

Action LookaheadActionTable::action(parse_state state, int lookahead) const {
    if (const auto* cell = findAction(state, lookahead)) {
        return *cell;
    }
    throw std::out_of_range {
        "No action for state " + std::to_string(state) + " lookahead " + std::to_string(lookahead)
    };
}

bool LookaheadActionTable::hasCorrectiveAction(parse_state state, int lookahead) const {
    const auto* cell = findAction(state, lookahead);
    return cell != nullptr && cell->isCorrective();
}

size_t LookaheadActionTable::size() const {
    return stateCount_;
}

void LookaheadActionTable::reserve(size_t stateCount) {
    lookaheadActions.reserve(stateCount * 4);
    errorCandidatesByState.reserve(stateCount);
}

void LookaheadActionTable::setStateCount(size_t stateCount) {
    stateCount_ = stateCount;
}

void LookaheadActionTable::setErrorCandidates(parse_state state, std::vector<int> candidates) {
    if (candidates.empty()) {
        errorCandidatesByState.erase(state);
        return;
    }
    if (state + 1 > stateCount_) {
        stateCount_ = state + 1;
    }
    errorCandidatesByState[state] = std::make_shared<const std::vector<int>>(std::move(candidates));
}

std::shared_ptr<const std::vector<int>> LookaheadActionTable::errorCandidates(parse_state state) const {
    const auto found = errorCandidatesByState.find(state);
    if (found == errorCandidatesByState.end()) {
        return nullptr;
    }
    return found->second;
}

std::vector<LookaheadActionTable::ExplicitAction> LookaheadActionTable::explicitActions() const {
    std::vector<ExplicitAction> rows;
    rows.reserve(lookaheadActions.size());
    for (const auto& entry : lookaheadActions) {
        rows.push_back({ entry.first.first, entry.first.second, entry.second });
    }
    return rows;
}

std::vector<LookaheadActionTable::ErrorRow> LookaheadActionTable::errorRows() const {
    std::vector<ErrorRow> rows;
    rows.reserve(errorCandidatesByState.size());
    for (const auto& entry : errorCandidatesByState) {
        rows.push_back({ entry.first, *entry.second });
    }
    return rows;
}

} // namespace parser
