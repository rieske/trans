#include "GeneratedParsingTable.h"

#include "Action.h"

#include <fstream>
#include <optional>

namespace parser {

GeneratedParsingTable::GeneratedParsingTable(const Grammar* grammar, const CanonicalCollectionStrategy& canonicalCollectionStrategy) :
    ParsingTable(grammar),
    firstTable { *this->grammar }
{
    CanonicalCollection canonicalCollection { firstTable, *this->grammar, canonicalCollectionStrategy };

    computeActionTable(canonicalCollection);
    computeGotoTable(canonicalCollection);
    computeErrorActions(canonicalCollection.stateCount());
}

GeneratedParsingTable::~GeneratedParsingTable() {
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
                    && (item.getLookaheads().front() == grammar->getEndSymbol())) {
                lookaheadActionTable.addAction(
                        currentState,
                        grammar->getEndSymbol(),
                        std::make_unique<AcceptAction>());
            } else {
                for (const auto& lookahead : item.getLookaheads()) {
                    lookaheadActionTable.addAction(
                            currentState,
                            lookahead,
                            std::make_unique<ReduceAction>(item.getProduction(), this));
                }
            }
        }
    }
}

void GeneratedParsingTable::computeGotoTable(const CanonicalCollection& canonicalCollection) {
    size_t stateCount = canonicalCollection.stateCount();
    for (parse_state state = 0; state < stateCount; ++state) {
        for (const auto& nonterminal : grammar->getNonterminalIDs()) {
            std::optional<parse_state> stateTo = canonicalCollection.findGoTo(state, nonterminal);
            if (stateTo) {
                gotoTable[state][nonterminal] = *stateTo;
            }
        }
    }
}

// FIXME: this is a mess
void GeneratedParsingTable::computeErrorActions(size_t stateCount) {
    for (std::size_t state = 0; state < stateCount; ++state) {
        int errorState = 0;
        int expected = grammar->symbolId(";");
        /*for (const auto& terminal : grammar->getTerminalIDs()) {
            try {
                auto& potentialAction = lookaheadActionTable.action(state, terminal);
                expected = terminal;
                //if (potentialAction is shift) {
                //} else if (potentialAction is reduce) {
                //}
            } catch (std::out_of_range&) {
            }
        }*/

        for (const auto& terminal : grammar->getTerminalIDs()) {
            if (!lookaheadActionTable.hasAction(state, terminal)) {
                lookaheadActionTable.addAction(
                        state,
                        terminal,
                        std::make_unique<ErrorAction>(errorState, expected, grammar));
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

    for (const auto& stateGotos : gotoTable) {
        for (const auto& nonterminalGotoState : stateGotos.second) {
            tableOutput << stateGotos.first << " " << nonterminalGotoState.first << " " << nonterminalGotoState.second << "\n";
        }
    }
}

} // namespace parser

