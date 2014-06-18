#include "GeneratedParsingTable.h"

#include <stddef.h>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "AcceptAction.h"
#include "CanonicalCollection.h"
#include "ErrorAction.h"
//#include "FirstTable.h"
//#include "GoTo.h"
#include "Grammar.h"
#include "GrammarSymbol.h"
#include "ReduceAction.h"
#include "ShiftAction.h"

using std::string;
using std::vector;
using std::unique_ptr;
using std::ostringstream;

using std::endl;

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

GeneratedParsingTable::GeneratedParsingTable(const Grammar& grammar) :
		firstTable { grammar.nonterminals },
		goTo { { firstTable } } {
	logger << grammar;

	CanonicalCollection canonicalCollection { firstTable };

	const vector<vector<LR1Item>> canonicalCollectionOfSetsOfItems = canonicalCollection.computeForGrammar(grammar);
	logCanonicalCollection(canonicalCollectionOfSetsOfItems);

	computeActionTable(canonicalCollectionOfSetsOfItems, grammar);
	computeGotoTable(canonicalCollectionOfSetsOfItems, grammar);
	computeErrorActions(grammar, canonicalCollectionOfSetsOfItems.size());
}

GeneratedParsingTable::~GeneratedParsingTable() {
}

void GeneratedParsingTable::computeActionTable(const vector<vector<LR1Item>>& canonicalCollectionOfSetsOfItems, const Grammar& grammar) {
	size_t stateCount = canonicalCollectionOfSetsOfItems.size();
	for (parse_state currentState = 0; currentState < stateCount; ++currentState) {    // for each state
		vector<LR1Item> setOfItemsForCurrentState = canonicalCollectionOfSetsOfItems.at(currentState);
		for (const auto& item : setOfItemsForCurrentState) {            // for each item in set
			vector<std::shared_ptr<const GrammarSymbol>> expectedSymbolsForItem = item.getExpectedSymbols();

			if (!expectedSymbolsForItem.empty()) {
				const auto nextExpectedSymbolForItem = expectedSymbolsForItem.at(0);
				if (nextExpectedSymbolForItem->isTerminal()) {
					vector<LR1Item> nextSetOfItems = goTo(setOfItemsForCurrentState, nextExpectedSymbolForItem);

					parse_state shiftToState = std::find(canonicalCollectionOfSetsOfItems.begin(), canonicalCollectionOfSetsOfItems.end(),
							nextSetOfItems) - canonicalCollectionOfSetsOfItems.begin();
					if (shiftToState < stateCount) {
						const auto expectedTerminal = nextExpectedSymbolForItem->getName();
						if (terminalActionTables[currentState].find(expectedTerminal) == terminalActionTables[currentState].end()) {
							terminalActionTables[currentState][expectedTerminal] = unique_ptr<Action> { new ShiftAction(shiftToState) };
						} else {
							auto& conflict = terminalActionTables[currentState].at(expectedTerminal);
							if (conflict->serialize() != ShiftAction { shiftToState }.serialize()) {
								ostringstream errorMessage;
								errorMessage << "Conflict with action: " << conflict->serialize() << " at state " << currentState
										<< " for a shift to state " << shiftToState;
								throw std::runtime_error(errorMessage.str());
							}
						}
					}
				}
			} else {     // dešinės pusės pabaiga
				if ((item.getDefiningSymbol() == grammar.startSymbol) && (item.getLookaheads().at(0) == grammar.endSymbol)) {
					terminalActionTables[currentState][grammar.endSymbol->getName()] = unique_ptr<Action> { new AcceptAction() };
				} else {
					for (const auto lookahead : item.getLookaheads()) {
						const auto lookaheadTerminal = lookahead->getName();
						if (terminalActionTables[currentState].find(lookaheadTerminal) == terminalActionTables[currentState].end()) {
							terminalActionTables[currentState][lookaheadTerminal] =
									unique_ptr<Action> { new ReduceAction(item, &gotoTable) };
						} else {
							auto& conflict = terminalActionTables[currentState].at(lookaheadTerminal);
							ostringstream errorMessage;
							errorMessage << "Conflict with action: " << conflict->serialize() << " at state " << currentState
									<< " for a reduce with rule " << item.productionStr();
							throw std::runtime_error(errorMessage.str());
						}
					}
				}
			}
		}
	}
}

void GeneratedParsingTable::computeGotoTable(const vector<vector<LR1Item>>& canonicalCollectionOfSetsOfItems, const Grammar& grammar) {
	size_t stateCount = canonicalCollectionOfSetsOfItems.size();
	for (size_t state = 0; state < stateCount; ++state) {
		vector<LR1Item> setOfItems = canonicalCollectionOfSetsOfItems.at(state);
		for (auto& nonterminal : grammar.nonterminals) {
			vector<LR1Item> nextSetOfItems = goTo(setOfItems, nonterminal);

			parse_state gotoState = std::find(canonicalCollectionOfSetsOfItems.begin(), canonicalCollectionOfSetsOfItems.end(),
					nextSetOfItems) - canonicalCollectionOfSetsOfItems.begin();
			if (gotoState < stateCount) {
				gotoTable[state][nonterminal] = gotoState;
			}
		}
	}
}

// FIXME: this is fucked
void GeneratedParsingTable::computeErrorActions(const Grammar& grammar, size_t stateCount) {
	std::shared_ptr<const GrammarSymbol> expected;
	string forge_token;
	for (int state = 0; state < stateCount; state++) {        // for each state
		unsigned term_size = 9999;
		forge_token.clear();
		int errorState = 0;
		for (auto& terminal : grammar.terminals) { // surandam galimą teisingą veiksmą
			try {
				auto& error_action = action(state, terminal->getName());
				//errorState = error_action.getState();
				if (terminal->getName().size() < term_size) {
					expected = terminal;
					term_size = terminal->getName().size();
					//if (error_action.which() == 'r') {
					//	forge_token = terminal->getName();
					//}
				}
			} catch (std::out_of_range&) {
			}
		}
		if (expected) {
			try {
				auto& error_action = action(state, expected->getName());
				//errorState = error_action.getState();
			} catch (std::out_of_range&) {
			}
		}

		for (auto& terminal : grammar.terminals) { // for each terminal
			try {
				action(state, terminal->getName());
			} catch (std::out_of_range&) {
				terminalActionTables[state][terminal->getName()] = unique_ptr<Action> { new ErrorAction(errorState, forge_token,
						expected->getName()) };
			}
		}
	}
}

void GeneratedParsingTable::logCanonicalCollection(const std::vector<std::vector<LR1Item>>& canonicalCollectionOfSetsOfItems) const {
	logger << "\n*********************";
	logger << "\nCanonical collection:\n";
	logger << "*********************\n";
	int setNo { 0 };
	for (const auto& setOfItems : canonicalCollectionOfSetsOfItems) {
		logger << "Set " << setNo++ << ":\n";
		for (const auto& item : setOfItems) {
			logger << item;
		}
		logger << "\n";
	}
}

void GeneratedParsingTable::output_table(const Grammar& grammar) const {
	std::ofstream table_out { "logs/parsing_table" };
	if (!table_out.is_open()) {
		throw std::runtime_error { "Unable to create parsing table output file!\n" };
	}

	size_t stateCount = terminalActionTables.size();
	table_out << stateCount << endl;
	table_out << "\%\%" << endl;
	for (int i = 0; i < stateCount; i++) {
		for (auto& terminal : grammar.terminals) {
			auto& act = action(i, terminal->getName());
			table_out << act.serialize() << "\n";
		}
	}
	table_out << "\%\%" << endl;
	for (int i = 0; i < stateCount; i++) {
		for (auto& nonterminal : grammar.nonterminals) {
			try {
				int state = go_to(i, nonterminal);
				table_out << "g" << " " << state << "\n";
			} catch (std::out_of_range&) {
				table_out << "0 0" << "\n";
			}
		}
	}
	table_out.close();
}
