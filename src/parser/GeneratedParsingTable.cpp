#include "GeneratedParsingTable.h"

#include "Action.h"
#include "FirstTable.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <utility>
#include <vector>

namespace parser {

GeneratedParsingTable::GeneratedParsingTable(const Grammar* grammar, AutomatonKind kind) :
    ParsingTable(grammar)
{
    FirstTable first { *this->grammar };
    CanonicalCollection canonicalCollection { first, *this->grammar, kind };

    computeActionTable(canonicalCollection);
    computeGotoTable(canonicalCollection);
    computeErrorActions(canonicalCollection.stateCount());
}

void GeneratedParsingTable::computeActionTable(const CanonicalCollection& canonicalCollection) {
    size_t stateCount = canonicalCollection.stateCount();
    lookaheadActionTable.reserve(stateCount);
    lookaheadActionTable.setStateCount(stateCount);
    const int topRuleId = grammar->getTopRule().getId();
    const int endSymbol = grammar->getEndSymbol();
    for (parse_state currentState = 0; currentState < stateCount; ++currentState) {
        const auto& setOfItemsForCurrentState = canonicalCollection.setOfItemsAtState(currentState);
        for (const auto& item : setOfItemsForCurrentState) {
            if (item.hasUnvisitedSymbols()) {
                const auto nextExpectedSymbolForItem = item.nextUnvisitedSymbol();
                if (grammar->isTerminal(nextExpectedSymbolForItem)) {
                    lookaheadActionTable.addAction(
                            currentState,
                            nextExpectedSymbolForItem,
                            Action::shift(canonicalCollection.goTo(currentState, nextExpectedSymbolForItem)));
                }
            } else if ((item.getProduction().getId() == topRuleId)
                    && item.hasLookahead(endSymbol, *grammar)) {
                lookaheadActionTable.addAction(
                        currentState,
                        endSymbol,
                        Action::accept());
            } else {
                const Production& production = item.getProduction();
                const auto& lookaheadBits = item.lookaheads();
                const std::size_t terminalCount = grammar->terminalCount();
                for (std::size_t bit = 0; bit < terminalCount; ++bit) {
                    if (lookaheadBits.test(bit)) {
                        lookaheadActionTable.addAction(
                                currentState,
                                grammar->terminalIdFromBit(bit),
                                Action::reduce(production, this));
                    }
                }
            }
        }
    }
}

void GeneratedParsingTable::computeGotoTable(const CanonicalCollection& canonicalCollection) {
    const auto& transitions = canonicalCollection.computedTransitions();
    gotoTable.reserve(transitions.size());
    // Copy only nonterminal transitions; terminal gotos are shifts in the action table.
    for (const auto& entry : transitions) {
        if (!grammar->isTerminal(entry.first.second)) {
            gotoTable[entry.first] = entry.second;
        }
    }
}

void GeneratedParsingTable::computeErrorActions(size_t stateCount) {
    const auto& terminals = grammar->getTerminalIDs();
    for (std::size_t state = 0; state < stateCount; ++state) {
        std::vector<int> candidates;
        for (const auto candidate : terminals) {
            if (lookaheadActionTable.hasCorrectiveAction(state, candidate)) {
                candidates.push_back(candidate);
            }
        }
        if (candidates.empty()) {
            // Registers grammar for bare errors; no per-state empty candidate storage.
            lookaheadActionTable.setErrorCandidates(state, nullptr, grammar);
        } else {
            lookaheadActionTable.setErrorCandidates(
                    state,
                    std::make_shared<const std::vector<int>>(std::move(candidates)),
                    grammar);
        }
    }
}

void GeneratedParsingTable::persistToFile(std::string fileName) const {
    std::ofstream tableOutput { fileName };
    if (!tableOutput.is_open()) {
        throw std::runtime_error { "Unable to create parsing table output file! fileName: " + fileName + "\n" };
    }

    size_t stateCount = lookaheadActionTable.size();
    tableOutput << stateCount << std::endl;
    tableOutput << "%%" << std::endl;
    for (std::size_t i = 0; i < stateCount; i++) {
        for (auto& terminal : grammar->getTerminalIDs()) {
            tableOutput << lookaheadActionTable.action(i, terminal).serialize() << "\n";
        }
    }
    tableOutput << "%%" << std::endl;

    // Stable order for checked-in table identity tests.
    std::vector<std::pair<StateSymbolKey, parse_state>> gotos(gotoTable.begin(), gotoTable.end());
    std::sort(gotos.begin(), gotos.end(), [](const auto& left, const auto& right) {
        if (left.first.first != right.first.first) {
            return left.first.first < right.first.first;
        }
        return left.first.second < right.first.second;
    });
    for (const auto& entry : gotos) {
        tableOutput << entry.first.first << " " << entry.first.second << " " << entry.second << "\n";
    }
}

} // namespace parser
