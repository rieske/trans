#include "ParsingTable.h"

#include <stddef.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "AcceptAction.h"
#include "BNFReader.h"
#include "CanonicalCollection.h"
#include "ErrorAction.h"
#include "FirstTable.h"
#include "GoTo.h"
#include "Grammar.h"
#include "GrammarSymbol.h"
#include "ReduceAction.h"
#include "ShiftAction.h"

using std::cerr;
using std::endl;
using std::ifstream;
using std::istream;
using std::istringstream;
using std::ostringstream;
using std::string;
using std::vector;
using std::map;
using std::unique_ptr;

ParsingTable::ParsingTable(Logger logger) :
		logger { logger } {
	BNFReader bnfReader { "grammar.bnf" };

	grammar = new Grammar { bnfReader.getGrammar() };

	string parsingTableFilename = "parsing_table";
	ifstream parsingTableStream { parsingTableFilename };
	if (!parsingTableStream.is_open()) {
		throw std::runtime_error("could not open parsing table configuration file for reading. Filename: " + parsingTableFilename);
	}
	read_table(parsingTableStream);
	parsingTableStream.close();
}

ParsingTable::ParsingTable(const string bnfFileName, Logger logger) :
		logger { logger } {
	BNFReader bnfReader { bnfFileName };

	grammar = new Grammar { bnfReader.getGrammar() };

	firstTable = std::unique_ptr<FirstTable> { new FirstTable { grammar->nonterminals } };
	goTo = std::unique_ptr<GoTo> { new GoTo { { *firstTable } } };
	CanonicalCollection canonicalCollection { *firstTable };

	items = canonicalCollection.computeForGrammar(*grammar);
	state_count = items.size();
	action_table = new map<std::string, unique_ptr<Action>> [state_count];

	fill_actions(items);
	fill_goto(items);
	fill_errors();
}

ParsingTable::~ParsingTable() {
	delete[] action_table;
	delete grammar;
}

void ParsingTable::read_table(istream& table) {
	table >> state_count;
	string delim;
	table >> delim;
	if (delim != "\%\%") {
		throw std::runtime_error("error in parsing table configuration file: \%\% delimiter expected");
	}
	action_table = new map<std::string, unique_ptr<Action>> [state_count];

	// pildom action lentelę
	for (int stateNumber = 0; stateNumber < state_count; ++stateNumber) {
		for (const auto& terminal : grammar->terminals) {
			char type;
			table >> type;
			parse_state state;
			table >> state;
			switch (type) {
			case 's': {
				action_table[stateNumber][terminal->getName()] = unique_ptr<Action> { new ShiftAction { state, logger } };
				continue;
			}
			case 'r': {
				size_t nonterminalId;
				size_t productionId;
				table >> nonterminalId >> productionId;
				action_table[stateNumber][terminal->getName()] = unique_ptr<Action> { new ReduceAction { grammar->getReductionById(
						nonterminalId, productionId), &goto_table, logger } };
				continue;
			}
			case 'a':
				action_table[stateNumber][grammar->endSymbol->getName()] = unique_ptr<Action> { new AcceptAction { logger } };
				continue;
			case 'e': {
				string forge;
				string expected;
				table >> forge >> expected;
				continue;
			}
			default:
				throw std::runtime_error("Error in parsing table configuration file: invalid action type: " + type);
			}
		}
	}
	fill_errors();

	table >> delim;
	if (delim != "\%\%") {
		throw std::runtime_error("error in parsing table configuration file: \%\% delimiter expected");
	}

	// pildom goto lentelę
	for (int stateNumber = 0; stateNumber < state_count; ++stateNumber) {
		for (const auto& nonterminal : grammar->nonterminals) {
			char type;
			parse_state state;
			table >> type >> state;
			if (type == '0') {
				continue;
			}
			goto_table[stateNumber][nonterminal] = state;
		}
	}
}

void ParsingTable::log(std::ostream &out) const {
	out << "Grammar:\n" << *grammar;

	if (!items.empty()) {
		out << "\n*********************";
		out << "\nCanonical collection:\n";
		out << "*********************\n";
		int setNo { 0 };
		for (const auto& setOfItems : items) {
			out << "Set " << setNo++ << ":\n";
			for (const auto& item : setOfItems) {
				out << item;
			}
			out << "\n";
		}
	}
}

void ParsingTable::output_table() const {
	std::ofstream table_out { "logs/parsing_table" };
	if (!table_out.is_open()) {
		throw std::runtime_error { "Unable to create parsing table output file!\n" };
	}

	table_out << state_count << endl;
	table_out << "\%\%" << endl;
	for (int i = 0; i < state_count; i++) {
		for (auto& terminal : grammar->terminals) {
			auto& act = action(i, terminal->getName());
			table_out << act.describe() << "\n";
		}
	}
	table_out << "\%\%" << endl;
	for (int i = 0; i < state_count; i++) {
		for (auto& nonterminal : grammar->nonterminals) {
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

Action& ParsingTable::action(parse_state state, string terminalId) const {
	return *action_table[state].at(terminalId);
}

parse_state ParsingTable::go_to(parse_state state, std::shared_ptr<const GrammarSymbol> nonterminal) const {
	return goto_table.at(state).at(nonterminal);
}

int ParsingTable::fill_actions(vector<vector<LR1Item>> C) {
	for (int state = 0; state < state_count; ++state) {    // for each state
		vector<LR1Item> set = C.at(state);
		for (const auto& item : set) {            // for each item in set
			vector<std::shared_ptr<const GrammarSymbol>> expected = item.getExpected();

			if (expected.size()) {
				if (expected.at(0)->isTerminal()) {
					vector<LR1Item> gt = (*goTo)(C.at(state), expected.at(0));
					if (!gt.empty()) {
						for (int actionState = 0; actionState < state_count; actionState++) {
							// XXX:
							if (C.at(actionState) == gt) {       // turim shift
								if (action_table[state].find(expected.at(0)->getName()) == action_table[state].end()) {
									action_table[state][expected.at(0)->getName()] = unique_ptr<Action> { new ShiftAction { actionState,
											logger } };
								} else {
									auto& conflict = *action_table[state].at(expected.at(0)->getName());
									if (conflict.describe() != ShiftAction { actionState, logger }.describe()) {
										ostringstream errorMessage;
										errorMessage << "Conflict with action: " << conflict.describe() << " at state " << state
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
				if ((item.getDefiningSymbol() == grammar->startSymbol) && (item.getLookaheads().at(0) == grammar->endSymbol)
						&& (expected.empty())) {
					action_table[state][grammar->endSymbol->getName()] = unique_ptr<Action> { new AcceptAction { logger } };
				} else {
					for (size_t j = 0; j < item.getLookaheads().size(); j++) {
						if (action_table[state].find(item.getLookaheads().at(j)->getName()) == action_table[state].end()) {
							action_table[state][item.getLookaheads().at(j)->getName()] = unique_ptr<Action> { new ReduceAction { item,
									&goto_table, logger } };
						} else {
							auto& conflict = *action_table[state].at(item.getLookaheads().at(j)->getName());
							ostringstream errorMessage;
							errorMessage << "Conflict with action: " << conflict.describe() << " at state " << state
									<< " for a reduce with rule " << item.productionStr();
							throw std::runtime_error(errorMessage.str());
						}
					}
				}
			}
		}
	}
	return 0;
}

int ParsingTable::fill_goto(vector<vector<LR1Item>> C) {
	for (int state = 0; state < state_count; ++state) {     // for each state
		vector<LR1Item> set = C.at(state);
		for (auto& nonterminal : grammar->nonterminals) {
			vector<LR1Item> gt = (*goTo)(set, nonterminal);
			if (!gt.empty()) {
				for (int gotoState = 0; gotoState < state_count; ++gotoState) {
					// XXX:
					if (C.at(gotoState) == gt) {
						goto_table[state][nonterminal] = gotoState;
					}
				}
			}
		}
	}
	return 0;
}

// FIXME: this is fucked
void ParsingTable::fill_errors() {
	std::shared_ptr<const GrammarSymbol> expected;
	string forge_token;
	for (int state = 0; state < state_count; state++) {        // for each state
		unsigned term_size = 9999;
		forge_token.clear();
		int errorState = 0;
		for (auto& terminal : grammar->terminals) { // surandam galimą teisingą veiksmą
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

		for (auto& terminal : grammar->terminals) { // for each terminal
			try {
				action(state, terminal->getName());
			} catch (std::out_of_range&) {
				action_table[state][terminal->getName()] = unique_ptr<Action> {
						new ErrorAction { errorState, forge_token, expected, logger } };
			}
		}
	}
}
