#include "GenerateParsingTable.h"

#include "Action.h"
#include "CompileParsingTable.h"
#include "FirstTable.h"

#include <unordered_map>
#include <vector>

namespace parser {
namespace {

void computeActionTable(const CanonicalCollection& canonicalCollection,
        LookaheadActionTable& actions, const Grammar& grammar) {
    const size_t stateCount = canonicalCollection.stateCount();
    actions.reserve(stateCount);
    actions.setStateCount(stateCount);
    const int topRuleId = grammar.getTopRule().getId();
    const int endSymbol = grammar.getEndSymbol();
    for (parse_state currentState = 0; currentState < stateCount; ++currentState) {
        const auto& setOfItemsForCurrentState = canonicalCollection.setOfItemsAtState(currentState);
        for (const auto& item : setOfItemsForCurrentState) {
            if (item.hasUnvisitedSymbols()) {
                const auto nextExpectedSymbolForItem = item.nextUnvisitedSymbol();
                if (grammar.isTerminal(nextExpectedSymbolForItem)) {
                    actions.addAction(
                            currentState,
                            nextExpectedSymbolForItem,
                            Action::shift(canonicalCollection.goTo(currentState, nextExpectedSymbolForItem)));
                }
            } else if ((item.getProduction().getId() == topRuleId)
                    && item.hasLookahead(endSymbol, grammar)) {
                actions.addAction(
                        currentState,
                        endSymbol,
                        Action::accept());
            } else {
                const Production& production = item.getProduction();
                const auto& lookaheadBits = item.lookaheads();
                const std::size_t terminalCount = grammar.terminalCount();
                for (std::size_t bit = 0; bit < terminalCount; ++bit) {
                    if (lookaheadBits.test(bit)) {
                        actions.addAction(
                                currentState,
                                grammar.terminalIdFromBit(bit),
                                Action::reduce(production));
                    }
                }
            }
        }
    }
}

void computeGotoTable(const CanonicalCollection& canonicalCollection,
        std::unordered_map<StateSymbolKey, parse_state, StateSymbolHash>& gotos,
        const Grammar& grammar) {
    const auto& transitions = canonicalCollection.computedTransitions();
    gotos.reserve(transitions.size());
    for (const auto& entry : transitions) {
        if (!grammar.isTerminal(entry.first.second)) {
            gotos[entry.first] = entry.second;
        }
    }
}

void computeErrorActions(size_t stateCount, LookaheadActionTable& actions, const Grammar& grammar) {
    const auto& terminals = grammar.getTerminalIDs();
    for (std::size_t state = 0; state < stateCount; ++state) {
        std::vector<int> candidates;
        for (const auto candidate : terminals) {
            if (actions.hasCorrectiveAction(state, candidate)) {
                candidates.push_back(candidate);
            }
        }
        if (!candidates.empty()) {
            actions.setErrorCandidates(state, std::move(candidates));
        }
    }
}

} // namespace

ParsingTable generateParsingTable(const Grammar* grammar, AutomatonKind kind) {
    FirstTable first { *grammar };
    CanonicalCollection canonicalCollection { first, *grammar, kind };
    LookaheadActionTable actions;
    std::unordered_map<StateSymbolKey, parse_state, StateSymbolHash> gotos;
    computeActionTable(canonicalCollection, actions, *grammar);
    computeGotoTable(canonicalCollection, gotos, *grammar);
    computeErrorActions(canonicalCollection.stateCount(), actions, *grammar);
    return compileParsingTable(actions, gotos, *grammar);
}

} // namespace parser
