#include "GeneratedParsingTable.h"

#include <fstream>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "AcceptAction.h"
#include "CanonicalCollection.h"
#include "ErrorAction.h"
#include "Grammar.h"
#include "GrammarSymbol.h"
#include "LookaheadActionTable.h"
#include "ReduceAction.h"
#include "ShiftAction.h"

using std::string;
using std::vector;
using std::unique_ptr;

using std::endl;

GeneratedParsingTable::GeneratedParsingTable(const Grammar* grammar) :
		ParsingTable(grammar),
		firstTable { this->grammar->getNonterminals() },
		goTo { { firstTable } } {

	CanonicalCollection canonicalCollection { firstTable, *this->grammar };

	computeActionTable(canonicalCollection);
	computeGotoTable(canonicalCollection);
	computeErrorActions(canonicalCollection.stateCount());
}

GeneratedParsingTable::~GeneratedParsingTable() {
}

void GeneratedParsingTable::computeActionTable(const CanonicalCollection& canonicalCollection) {
	size_t stateCount = canonicalCollection.stateCount();
	for (parse_state currentState = 0; currentState < stateCount; ++currentState) {
		vector<LR1Item> setOfItemsForCurrentState = canonicalCollection.setOfItemsAtState(currentState);
		for (const auto& item : setOfItemsForCurrentState) {
			vector<const GrammarSymbol*> expectedSymbolsForItem = item.getExpectedSymbols();

			if (!expectedSymbolsForItem.empty()) {
				const auto nextExpectedSymbolForItem = expectedSymbolsForItem.at(0);
				if (nextExpectedSymbolForItem->isTerminal()) {
					vector<LR1Item> nextSetOfItems = goTo(setOfItemsForCurrentState, nextExpectedSymbolForItem);
					if (canonicalCollection.contains(nextSetOfItems)) {
						lookaheadActionTable.addAction(currentState, nextExpectedSymbolForItem->getSymbol(), unique_ptr<Action> {
								new ShiftAction(canonicalCollection.stateFor(nextSetOfItems)) });
					}
				}
			} else if ((item.getDefiningSymbol() == grammar->getStartSymbol()->getSymbol())
					&& (item.getLookaheads().at(0) == grammar->getEndSymbol())) {
				lookaheadActionTable.addAction(currentState, grammar->getEndSymbol()->getSymbol(),
						unique_ptr<Action> { new AcceptAction() });
			} else {
				for (const auto lookahead : item.getLookaheads()) {
					lookaheadActionTable.addAction(currentState, lookahead->getSymbol(),
							unique_ptr<Action> { new ReduceAction(item, this) });
				}
			}
		}
	}
}

void GeneratedParsingTable::computeGotoTable(const CanonicalCollection& canonicalCollection) {
	size_t stateCount = canonicalCollection.stateCount();
	for (size_t state = 0; state < stateCount; ++state) {
		vector<LR1Item> setOfItems = canonicalCollection.setOfItemsAtState(state);
		for (auto& nonterminal : grammar->getNonterminals()) {
			vector<LR1Item> nextSetOfItems = goTo(setOfItems, nonterminal);
			if (canonicalCollection.contains(nextSetOfItems)) {
				gotoTable[state][nonterminal->getSymbol()] = canonicalCollection.stateFor(nextSetOfItems);
			}
		}
	}
}

// FIXME: this is fucked
void GeneratedParsingTable::computeErrorActions(size_t stateCount) {
	const GrammarSymbol* expected;
	string forge_token;
	for (int state = 0; state < stateCount; state++) {        // for each state
		unsigned term_size = 9999;
		forge_token.clear();
		int errorState = 0;
		for (auto& terminal : grammar->getTerminals()) { // surandam galimą teisingą veiksmą
			try {
				auto& error_action = action(state, terminal->getSymbol());
				//errorState = error_action.getState();
				if (terminal->getSymbol().size() < term_size) {
					expected = terminal;
					term_size = terminal->getSymbol().size();
					//if (error_action.which() == 'r') {
					//	forge_token = terminal->getName();
					//}
				}
			} catch (std::out_of_range&) {
			}
		}
		if (expected) {
			try {
				auto& error_action = action(state, expected->getSymbol());
				//errorState = error_action.getState();
			} catch (std::out_of_range&) {
			}
		}

		for (auto& terminal : grammar->getTerminals()) { // for each terminal
			try {
				action(state, terminal->getSymbol());
			} catch (std::out_of_range&) {
				lookaheadActionTable.addAction(state, terminal->getSymbol(),
						unique_ptr<Action> { new ErrorAction(errorState, forge_token, expected->getSymbol()) });
			}
		}
	}
}

void GeneratedParsingTable::persistToFile(string fileName) const {
	std::ofstream table_out { fileName };
	if (!table_out.is_open()) {
		throw std::runtime_error { "Unable to create parsing table output file!\n" };
	}

	size_t stateCount = lookaheadActionTable.size();
	table_out << stateCount << endl;
	table_out << "\%\%" << endl;
	for (int i = 0; i < stateCount; i++) {
		for (auto& terminal : grammar->getTerminals()) {
			auto& act = action(i, terminal->getSymbol());
			table_out << act.serialize() << "\n";
		}
	}
	table_out << "\%\%" << endl;
	for (int i = 0; i < stateCount; i++) {
		for (auto& nonterminal : grammar->getNonterminals()) {
			try {
				int state = go_to(i, nonterminal->getSymbol());
				table_out << "g" << " " << state << "\n";
			} catch (std::out_of_range&) {
				table_out << "0 0" << "\n";
			}
		}
	}
}
