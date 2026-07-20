#include "GeneratedParsingTable.h"

#include "Action.h"
#include "FirstTable.h"

#include <algorithm>
#include <fstream>
#include <optional>
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
    for (parse_state currentState = 0; currentState < stateCount; ++currentState) {
        const auto& setOfItemsForCurrentState = canonicalCollection.setOfItemsAtState(currentState);
        for (const auto& item : setOfItemsForCurrentState) {
            if (item.hasUnvisitedSymbols()) {
                const auto nextExpectedSymbolForItem = item.nextUnvisitedSymbol();
                if (grammar->isTerminal(nextExpectedSymbolForItem)) {
                    lookaheadActionTable.addAction(
                            currentState,
                            nextExpectedSymbolForItem,
                            std::make_unique<ShiftAction>(canonicalCollection.goTo(currentState, nextExpectedSymbolForItem)));
                }
            } else if ((item.getProduction().getId() == grammar->getTopRule().getId())
                    && item.hasLookahead(grammar->getEndSymbol(), *grammar)) {
                lookaheadActionTable.addAction(
                        currentState,
                        grammar->getEndSymbol(),
                        std::make_unique<AcceptAction>());
            } else {
                // Dense bitset path: iterate terminal bits instead of materializing a vector.
                const auto& lookaheadBits = item.lookaheads();
                const std::size_t terminalCount = grammar->terminalCount();
                for (std::size_t bit = 0; bit < terminalCount; ++bit) {
                    if (lookaheadBits.test(bit)) {
                        lookaheadActionTable.addAction(
                                currentState,
                                grammar->terminalIdFromBit(bit),
                                std::make_unique<ReduceAction>(item.getProduction(), this));
                    }
                }
            }
        }
    }
}

void GeneratedParsingTable::computeGotoTable(const CanonicalCollection& canonicalCollection) {
    const auto& transitions = canonicalCollection.computedTransitions();
    // Nested goto map retained until the action-table layer flattens keys.
    for (const auto& entry : transitions) {
        if (!grammar->isTerminal(entry.first.second)) {
            gotoTable[entry.first.first][entry.first.second] = entry.second;
        }
    }
}

void GeneratedParsingTable::computeErrorActions(size_t stateCount) {
    int errorState = 0;
    for (std::size_t state = 0; state < stateCount; ++state) {
        for (const auto& terminal : grammar->getTerminalIDs()) {
            if (!lookaheadActionTable.hasAction(state, terminal)) {
                std::vector<int> candidateTerminals;
                for (const auto& candidate : grammar->getTerminalIDs()) {
                    if (candidate != terminal
                            && lookaheadActionTable.hasAction(state, candidate)
                            && lookaheadActionTable.action(state, candidate).isCorrective()) {
                        candidateTerminals.push_back(candidate);
                    }
                }
                lookaheadActionTable.addAction(state, terminal, std::make_unique<ErrorAction>(errorState, candidateTerminals, grammar));
            }
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
            auto& act = lookaheadActionTable.action(i, terminal);
            tableOutput << act.serialize() << "\n";
        }
    }
    tableOutput << "%%" << std::endl;

    // Stable order for checked-in table identity tests.
    std::vector<std::tuple<parse_state, int, parse_state>> gotos;
    for (const auto& stateGotos : gotoTable) {
        for (const auto& nonterminalGotoState : stateGotos.second) {
            gotos.emplace_back(stateGotos.first, nonterminalGotoState.first, nonterminalGotoState.second);
        }
    }
    std::sort(gotos.begin(), gotos.end());
    for (const auto& entry : gotos) {
        tableOutput << std::get<0>(entry) << " " << std::get<1>(entry) << " " << std::get<2>(entry) << "\n";
    }
}

} // namespace parser
