#include "GeneratedParsingTable.h"

#include <fstream>
#include "AcceptAction.h"
#include "ErrorAction.h"
#include "ReduceAction.h"
#include "ShiftAction.h"

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
                if (nextExpectedSymbolForItem.isTerminal()) {
                    lookaheadActionTable.addAction(
                            currentState,
                            nextExpectedSymbolForItem.getId(),
                            std::make_unique<ShiftAction>(canonicalCollection.goTo(currentState, nextExpectedSymbolForItem.getId())));
                }
            } else if ((item.getDefiningSymbol() == grammar->getStartSymbol()) && (item.getLookaheads().front() == grammar->getEndSymbol())) {
                lookaheadActionTable.addAction(
                        currentState,
                        grammar->getEndSymbol().getId(),
                        std::make_unique<AcceptAction>());
            } else {
                for (const auto& lookahead : item.getLookaheads()) {
                    lookaheadActionTable.addAction(
                            currentState,
                            lookahead.getId(),
                            std::make_unique<ReduceAction>(item.getProduction(), this));
                }
            }
        }
    }
}

void GeneratedParsingTable::computeGotoTable(const CanonicalCollection& canonicalCollection) {
    size_t stateCount = canonicalCollection.stateCount();
    for (parse_state state = 0; state < stateCount; ++state) {
        std::vector<LR1Item> setOfItems = canonicalCollection.setOfItemsAtState(state);
        for (auto& nonterminal : grammar->getNonterminals()) {
            try {
                auto stateTo = canonicalCollection.goTo(state, nonterminal.getId());
                gotoTable[state][nonterminal.getId()] = stateTo;
            } catch (std::out_of_range&) {
            }
        }
    }
}

// FIXME: this is a mess
void GeneratedParsingTable::computeErrorActions(size_t stateCount) {
    std::unique_ptr<GrammarSymbol> expected;
    std::string forge_token;
    for (std::size_t state = 0; state < stateCount; ++state) {
        int termId = 9999;
        forge_token.clear();
        int errorState = 0;
        for (auto& terminal : grammar->getTerminals()) {
            try {
                //auto& error_action = lookaheadActionTable.action(state, terminal.getDefinition());
                //errorState = error_action.getState();
                if (terminal.getId() < termId) {
                    expected = std::make_unique<GrammarSymbol>(terminal);
                    termId = terminal.getId();
                    //if (error_action.which() == 'r') {
                    //	forge_token = terminal->getName();
                    //}
                }
            } catch (std::out_of_range&) {
            }
        }
        if (expected) {
            try {
                //auto& error_action = lookaheadActionTable.action(state, expected->getDefinition());
                //errorState = error_action.getState();
            } catch (std::out_of_range&) {
            }
        }

        for (auto& terminal : grammar->getTerminals()) {
            try {
                lookaheadActionTable.action(state, terminal.getId());
            } catch (std::out_of_range&) {
                lookaheadActionTable.addAction(
                        state,
                        terminal.getId(),
                        std::make_unique<ErrorAction>(errorState, forge_token, expected->getId(), grammar));
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
        for (auto& terminal : grammar->getTerminals()) {
            auto& act = lookaheadActionTable.action(i, terminal.getId());
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

void parser::GeneratedParsingTable::outputPretty(std::string fileName) const {
    std::ofstream tableOutput { fileName };
    if (!tableOutput.is_open()) {
        throw std::runtime_error { "Unable to create parsing table output file!\n" };
    }

    tableOutput << "\t";
    for (auto& terminal : grammar->getTerminals()) {
        tableOutput << terminal.getId() << "\t\t|\t";
    }
    tableOutput << "\n";
    for (std::size_t i = 0; i < lookaheadActionTable.size(); ++i) {
        tableOutput << i << "\t";
        for (auto& terminal : grammar->getTerminals()) {
            auto& act = lookaheadActionTable.action(i, terminal.getId());
            switch (act.serialize().at(0)) {
            case 'e':
                tableOutput << "e" << "\t\t|\t";
                break;
            case 'r':
                tableOutput << act.serialize() << "\t|\t";
                break;
            default:
                tableOutput << act.serialize() << "\t\t|\t";
            }
        }
        tableOutput << "\n";
    }
    tableOutput << "%%\n";
    for (const auto& stateGotos : gotoTable) {
        tableOutput << stateGotos.first << "\t";
        for (const auto& nonterminalGotoState : stateGotos.second) {
            tableOutput << nonterminalGotoState.first << " " << nonterminalGotoState.second << "\t";
        }
        tableOutput << "\n";
    }
}

} // namespace parser

