#include "GeneratedParsingTable.h"

#include <stddef.h>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "AcceptAction.h"
#include "CanonicalCollection.h"
#include "ErrorAction.h"
#include "FirstTable.h"
#include "GoTo.h"
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
	for (int state = 0; state < stateCount; ++state) {    // for each state
		vector<LR1Item> set = canonicalCollectionOfSetsOfItems.at(state);
		for (const auto& item : set) {            // for each item in set
			vector<std::shared_ptr<const GrammarSymbol>> expected = item.getExpected();

			if (expected.size()) {
				if (expected.at(0)->isTerminal()) {
					vector<LR1Item> gt = goTo(canonicalCollectionOfSetsOfItems.at(state), expected.at(0));
					if (!gt.empty()) {
						for (int actionState = 0; actionState < stateCount; actionState++) {
							// XXX:
							if (canonicalCollectionOfSetsOfItems.at(actionState) == gt) {       // turim shift
								if (terminalActionTables[state].find(expected.at(0)->getName()) == terminalActionTables[state].end()) {
									terminalActionTables[state][expected.at(0)->getName()] = unique_ptr<Action> { new ShiftAction(
											actionState) };
								} else {
									auto& conflict = terminalActionTables[state].at(expected.at(0)->getName());
									if (conflict->describe() != ShiftAction { actionState }.describe()) {
										ostringstream errorMessage;
										errorMessage << "Conflict with action: " << conflict->describe() << " at state " << state
												<< " for a shift to state " << actionState;
										throw std::runtime_error(errorMessage.str());
									}
								}
								break;
							}
						}
					}
				}
			} else {     // dešinės pusės pabaiga
				if ((item.getDefiningSymbol() == grammar.startSymbol) && (item.getLookaheads().at(0) == grammar.endSymbol)
						&& (expected.empty())) {
					terminalActionTables[state][grammar.endSymbol->getName()] = unique_ptr<Action> { new AcceptAction() };
				} else {
					for (size_t j = 0; j < item.getLookaheads().size(); j++) {
						if (terminalActionTables[state].find(item.getLookaheads().at(j)->getName()) == terminalActionTables[state].end()) {
							terminalActionTables[state][item.getLookaheads().at(j)->getName()] = unique_ptr<Action> { new ReduceAction(item,
									&gotoTable) };
						} else {
							auto& conflict = terminalActionTables[state].at(item.getLookaheads().at(j)->getName());
							ostringstream errorMessage;
							errorMessage << "Conflict with action: " << conflict->describe() << " at state " << state
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
	for (int state = 0; state < stateCount; ++state) {     // for each state
		vector<LR1Item> set = canonicalCollectionOfSetsOfItems.at(state);
		for (auto& nonterminal : grammar.nonterminals) {
			vector<LR1Item> gt = goTo(set, nonterminal);
			if (!gt.empty()) {
				for (int gotoState = 0; gotoState < stateCount; ++gotoState) {
					// XXX:
					if (canonicalCollectionOfSetsOfItems.at(gotoState) == gt) {
						gotoTable[state][nonterminal] = gotoState;
					}
				}
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
			table_out << act.describe() << "\n";
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
