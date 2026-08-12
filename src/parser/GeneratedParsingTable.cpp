#include "GeneratedParsingTable.h"

#include "Action.h"
#include "FirstTable.h"

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
        if (!candidates.empty()) {
            lookaheadActionTable.setErrorCandidates(state, std::move(candidates));
        }
    }
}

} // namespace parser
