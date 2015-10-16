#include "parser/GeneratedParsingTable.h"

#include <cstddef>
#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

#include "parser/AcceptAction.h"
#include "parser/CanonicalCollection.h"
#include "parser/ErrorAction.h"
#include "parser/Grammar.h"
#include "parser/GrammarSymbol.h"
#include "parser/LookaheadActionTable.h"
#include "parser/Production.h"
#include "parser/ReduceAction.h"
#include "parser/ShiftAction.h"

using std::string;
using std::vector;

using std::endl;

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
                            nextExpectedSymbolForItem.getDefinition(),
                            std::make_unique<ShiftAction>(canonicalCollection.goTo(currentState, nextExpectedSymbolForItem.getDefinition())));
                }
            } else if ((item.getDefiningSymbol() == grammar->getStartSymbol()) && (item.getLookaheads().front() == grammar->getEndSymbol())) {
                lookaheadActionTable.addAction(
                        currentState,
                        grammar->getEndSymbol().getDefinition(),
                        std::make_unique<AcceptAction>());
            } else {
                for (const auto lookahead : item.getLookaheads()) {
                    lookaheadActionTable.addAction(
                            currentState,
                            lookahead.getDefinition(),
                            std::make_unique<ReduceAction>(item.getProduction(), this));
                }
            }
        }
    }
}

void GeneratedParsingTable::computeGotoTable(const CanonicalCollection& canonicalCollection) {
    size_t stateCount = canonicalCollection.stateCount();
    for (parse_state state = 0; state < stateCount; ++state) {
        vector<LR1Item> setOfItems = canonicalCollection.setOfItemsAtState(state);
        for (auto& nonterminal : grammar->getNonterminals()) {
            try {
                auto stateTo = canonicalCollection.goTo(state, nonterminal.getDefinition());
                gotoTable[state][nonterminal.getDefinition()] = stateTo;
            } catch (std::out_of_range&) {
            }
        }
    }
}

// FIXME: this is fucked
void GeneratedParsingTable::computeErrorActions(size_t stateCount) {
    std::unique_ptr<GrammarSymbol> expected;
    string forge_token;
    for (std::size_t state = 0; state < stateCount; ++state) {        // for each state
        unsigned term_size = 9999;
        forge_token.clear();
        int errorState = 0;
        for (auto& terminal : grammar->getTerminals()) { // surandam galimą teisingą veiksmą
            try {
                auto& error_action = lookaheadActionTable.action(state, terminal.getDefinition());
                //errorState = error_action.getState();
                if (terminal.getDefinition().size() < term_size) {
                    expected = std::make_unique<GrammarSymbol>(terminal);
                    term_size = terminal.getDefinition().size();
                    //if (error_action.which() == 'r') {
                    //	forge_token = terminal->getName();
                    //}
                }
            } catch (std::out_of_range&) {
            }
        }
        if (expected) {
            try {
                auto& error_action = lookaheadActionTable.action(state, expected->getDefinition());
                //errorState = error_action.getState();
            } catch (std::out_of_range&) {
            }
        }

        for (auto& terminal : grammar->getTerminals()) { // for each terminal
            try {
                lookaheadActionTable.action(state, terminal.getDefinition());
            } catch (std::out_of_range&) {
                lookaheadActionTable.addAction(
                        state,
                        terminal.getDefinition(),
                        std::make_unique<ErrorAction>(errorState, forge_token, expected->getDefinition()));
            }
        }
    }
}

void GeneratedParsingTable::persistToFile(string fileName) const {
    std::ofstream tableOutput { fileName };
    if (!tableOutput.is_open()) {
        throw std::runtime_error { "Unable to create parsing table output file!\n" };
    }

    size_t stateCount = lookaheadActionTable.size();
    tableOutput << stateCount << endl;
    tableOutput << "\%\%" << endl;
    for (std::size_t i = 0; i < stateCount; i++) {
        for (auto& terminal : grammar->getTerminals()) {
            auto& act = lookaheadActionTable.action(i, terminal.getDefinition());
            tableOutput << act.serialize() << "\n";
        }
    }
    tableOutput << "\%\%" << endl;

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
        tableOutput << terminal.getDefinition() << "\t\t|\t";
    }
    tableOutput << "\n";
    for (std::size_t i = 0; i < lookaheadActionTable.size(); ++i) {
        tableOutput << i << "\t";
        for (auto& terminal : grammar->getTerminals()) {
            auto& act = lookaheadActionTable.action(i, terminal.getDefinition());
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
    tableOutput << "\%\%\n";
    for (const auto& stateGotos : gotoTable) {
        tableOutput << stateGotos.first << "\t";
        for (const auto& nonterminalGotoState : stateGotos.second) {
            tableOutput << nonterminalGotoState.first << " " << nonterminalGotoState.second << "\t";
        }
        tableOutput << "\n";
    }
}

}

