#include "ParsingTable.h"

#include "Action.h"
#include "ParsingTableFile.h"
#include "util/Logger.h"
#include "util/LogManager.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
static Logger& logger = LogManager::getComponentLogger(Component::PARSER);
} // namespace

namespace parser {
namespace {

bool actionRecordLess(const ParsingTableActionRecord& left, const ParsingTableActionRecord& right) {
    if (left.state != right.state) {
        return left.state < right.state;
    }
    return left.terminal < right.terminal;
}

bool errorRecordLess(const ParsingTableErrorRecord& left, const ParsingTableErrorRecord& right) {
    return left.state < right.state;
}

bool gotoRecordLess(const ParsingTableGotoRecord& left, const ParsingTableGotoRecord& right) {
    if (left.fromState != right.fromState) {
        return left.fromState < right.fromState;
    }
    return left.nonterminal < right.nonterminal;
}

} // namespace

ParsingTable::ParsingTable(const Grammar* grammar) :
		grammar { grammar } {
	logger << *this->grammar;
}

std::size_t ParsingTable::stateCount() const {
    return lookaheadActionTable.size();
}

void ParsingTable::persistToFile(const std::string& fileName) const {
    std::ofstream out { fileName };
    if (!out) {
        throw std::runtime_error { "Unable to create parsing table output file! fileName: " + fileName + "\n" };
    }

    std::vector<ParsingTableActionRecord> actions;
    for (const auto& row : lookaheadActionTable.explicitActions()) {
        actions.push_back({ row.state, row.lookahead, row.action.serialize() });
    }
    std::sort(actions.begin(), actions.end(), actionRecordLess);

    std::vector<ParsingTableErrorRecord> errors;
    for (const auto& row : lookaheadActionTable.errorRows()) {
        errors.push_back({ row.state, row.candidates });
    }
    std::sort(errors.begin(), errors.end(), errorRecordLess);

    std::vector<ParsingTableGotoRecord> gotos;
    gotos.reserve(gotoTable.size());
    for (const auto& entry : gotoTable) {
        gotos.push_back({ entry.first.first, entry.first.second, entry.second });
    }
    std::sort(gotos.begin(), gotos.end(), gotoRecordLess);

    ParsingTableWriter writer { out };
    writer.writeHeader(lookaheadActionTable.size());
    writer.writeActions(actions);
    writer.writeErrors(errors);
    writer.writeGotos(gotos);
}

void ParsingTable::loadFromFile(const std::string& fileName) {
    std::ifstream in { fileName };
    if (!in) {
        throw std::runtime_error(
                "could not open parsing table configuration file for reading. Filename: " + fileName);
    }

    ParsingTableReader reader { in };
    const std::size_t states = reader.readHeader();
    lookaheadActionTable.reserve(states);
    lookaheadActionTable.setStateCount(states);

    for (const auto& action : reader.readActions()) {
        lookaheadActionTable.addAction(
                action.state,
                action.terminal,
                Action::deserialize(action.serialized, *this, *grammar));
    }
    for (const auto& error : reader.readErrors()) {
        lookaheadActionTable.setErrorCandidates(error.state, error.candidates);
    }
    for (const auto& edge : reader.readGotos()) {
        gotoTable[{ edge.fromState, edge.nonterminal }] = edge.toState;
    }
}

Action ParsingTable::action(parse_state state, const scanner::Token& lookahead) const {
    static const auto kEmptyCandidates = std::make_shared<const std::vector<int>>();

    if (lookahead.symbolId < 0) {
        throw std::logic_error { "ParsingTable::action: lookahead is not a grammar terminal" };
    }
    if (const auto* cell = lookaheadActionTable.findAction(state, lookahead.symbolId)) {
        return *cell;
    }
    if (auto candidates = lookaheadActionTable.errorCandidates(state)) {
        return Action::error(0, std::move(candidates), grammar);
    }
    return Action::error(0, kEmptyCandidates, grammar);
}

parse_state ParsingTable::go_to(parse_state state, int nonterminal) const {
    return gotoTable.at({ state, nonterminal });
}

std::optional<parse_state> ParsingTable::tryGoTo(parse_state state, int nonterminal) const {
    auto found = gotoTable.find({ state, nonterminal });
    if (found == gotoTable.end()) {
        return std::nullopt;
    }
    return found->second;
}

} // namespace parser

