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

GeneratedParsingTable::GeneratedParsingTable(const Grammar* grammar) :
		ParsingTable { grammar },
		firstTable { this->grammar->getNonterminals() },
		goTo { { firstTable } } {

	CanonicalCollection canonicalCollection { firstTable };

	const vector<vector<LR1Item>> canonicalCollectionOfSetsOfItems = canonicalCollection.computeForGrammar(*this->grammar);
	logCanonicalCollection(canonicalCollectionOfSetsOfItems);

	computeActionTable(canonicalCollectionOfSetsOfItems);
	computeGotoTable(canonicalCollectionOfSetsOfItems);
	computeErrorActions(canonicalCollectionOfSetsOfItems.size());
}

GeneratedParsingTable::~GeneratedParsingTable() {
}

void GeneratedParsingTable::computeActionTable(const vector<vector<LR1Item>>& canonicalCollectionOfSetsOfItems) {
	size_t stateCount = canonicalCollectionOfSetsOfItems.size();
	for (parse_state currentState = 0; currentState < stateCount; ++currentState) {
		vector<LR1Item> setOfItemsForCurrentState = canonicalCollectionOfSetsOfItems.at(currentState);
		for (const auto& item : setOfItemsForCurrentState) {
			vector<const GrammarSymbol*> expectedSymbolsForItem = item.getExpectedSymbols();

			if (!expectedSymbolsForItem.empty()) {
				const auto nextExpectedSymbolForItem = expectedSymbolsForItem.at(0);
				if (nextExpectedSymbolForItem->isTerminal()) {
					vector<LR1Item> nextSetOfItems = goTo(setOfItemsForCurrentState, nextExpectedSymbolForItem);

					parse_state shiftToState = std::find(canonicalCollectionOfSetsOfItems.begin(), canonicalCollectionOfSetsOfItems.end(),
							nextSetOfItems) - canonicalCollectionOfSetsOfItems.begin();
					if (shiftToState < stateCount) {
						const auto expectedTerminal = nextExpectedSymbolForItem->getSymbol();
						if (lookaheadActions[currentState].find(expectedTerminal) == lookaheadActions[currentState].end()) {
							lookaheadActions[currentState][expectedTerminal] = unique_ptr<Action> { new ShiftAction(shiftToState) };
						} else {
							auto& conflict = lookaheadActions[currentState].at(expectedTerminal);
							if (conflict->serialize() != ShiftAction { shiftToState }.serialize()) {
								ostringstream errorMessage;
								errorMessage << "Conflict with action: " << conflict->serialize() << " at state " << currentState
										<< " for a shift to state " << shiftToState;
								throw std::runtime_error(errorMessage.str());
							}
						}
					}
				}
			} else {
				if ((item.getDefiningSymbol() == grammar->getStartSymbol()->getSymbol())
						&& (item.getLookaheads().at(0) == grammar->getEndSymbol())) {
					lookaheadActions[currentState][grammar->getEndSymbol()->getSymbol()] = unique_ptr<Action> { new AcceptAction() };
				} else {
					for (const auto lookahead : item.getLookaheads()) {
						const auto lookaheadTerminal = lookahead->getSymbol();
						if (lookaheadActions[currentState].find(lookaheadTerminal) == lookaheadActions[currentState].end()) {
							lookaheadActions[currentState][lookaheadTerminal] = unique_ptr<Action> { new ReduceAction(item, this) };
						} else {
							auto& conflict = lookaheadActions[currentState].at(lookaheadTerminal);
							ostringstream errorMessage;
							errorMessage << "Conflict with action: " << conflict->serialize() << " at state " << currentState
									<< " for a reduce with rule " << item.getProductionNumber() << " of item " << item;
							throw std::runtime_error(errorMessage.str());
						}
					}
				}
			}
		}
	}
}

void GeneratedParsingTable::computeGotoTable(const vector<vector<LR1Item>>& canonicalCollectionOfSetsOfItems) {
	size_t stateCount = canonicalCollectionOfSetsOfItems.size();
	for (size_t state = 0; state < stateCount; ++state) {
		vector<LR1Item> setOfItems = canonicalCollectionOfSetsOfItems.at(state);
		for (auto& nonterminal : grammar->getNonterminals()) {
			vector<LR1Item> nextSetOfItems = goTo(setOfItems, nonterminal);

			parse_state gotoState = std::find(canonicalCollectionOfSetsOfItems.begin(), canonicalCollectionOfSetsOfItems.end(),
					nextSetOfItems) - canonicalCollectionOfSetsOfItems.begin();
			if (gotoState < stateCount) {
				gotoTable[state][nonterminal->getSymbol()] = gotoState;
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
				lookaheadActions[state][terminal->getSymbol()] = unique_ptr<Action> { new ErrorAction(errorState, forge_token,
						expected->getSymbol()) };
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

void GeneratedParsingTable::persistToFile(string fileName) const {
	std::ofstream table_out { fileName };
	if (!table_out.is_open()) {
		throw std::runtime_error { "Unable to create parsing table output file!\n" };
	}

	size_t stateCount = lookaheadActions.size();
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
