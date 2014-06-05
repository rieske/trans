#include "ParsingTable.h"

#include <stddef.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "action.h"
#include "BNFReader.h"
#include "CanonicalCollection.h"
#include "FirstTable.h"
#include "GoTo.h"
#include "Grammar.h"
#include "GrammarSymbol.h"

using std::cerr;
using std::endl;
using std::ifstream;
using std::istream;
using std::istringstream;
using std::string;
using std::vector;
using std::map;
using std::unique_ptr;

ParsingTable::ParsingTable() {
	BNFReader bnfReader { "grammar.bnf" };

	grammar = new Grammar { bnfReader.getGrammar() };

	idToTerminalMappingTable = bnfReader.getIdToTerminalMappingTable();
	idToTerminalMappingTable[0] = grammar->endSymbol;

	string parsingTableFilename = "parsing_table";
	ifstream parsingTableStream { parsingTableFilename };
	if (!parsingTableStream.is_open()) {
		throw std::runtime_error("could not open parsing table configuration file for reading. Filename: " + parsingTableFilename);
	}
	read_table(parsingTableStream);
	parsingTableStream.close();
}

ParsingTable::ParsingTable(const string bnfFileName) {
	BNFReader bnfReader { bnfFileName };

	grammar = new Grammar { bnfReader.getGrammar() };

	idToTerminalMappingTable = bnfReader.getIdToTerminalMappingTable();
	idToTerminalMappingTable[0] = grammar->endSymbol;

	firstTable = std::unique_ptr<FirstTable> { new FirstTable { grammar->nonterminals } };
	goTo = std::unique_ptr<GoTo> { new GoTo { { *firstTable } } };
	CanonicalCollection canonicalCollection(*firstTable);

	items = canonicalCollection.computeForGrammar(*grammar);
	state_count = items.size();
	action_table = new map<std::shared_ptr<const GrammarSymbol>, unique_ptr<Action>> [state_count];

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
	action_table = new map<std::shared_ptr<const GrammarSymbol>, unique_ptr<Action>> [state_count];

	// pildom action lentelę
	for (int stateNumber = 0; stateNumber < state_count; ++stateNumber) {
		for (const auto& terminal : idToTerminalMappingTable) {
			string actionDefinition;
			table >> actionDefinition;
			istringstream actionDefinitionStream { actionDefinition };
			char type;
			actionDefinitionStream >> type;
			parse_state state;
			actionDefinitionStream >> state;

			switch (type) {
			case 's': {
				action_table[stateNumber][terminal.second] = unique_ptr<Action> { new Action('s', state) };
				continue;
			}
			case 'r': {
				char commaDelim;
				size_t nonterminalId;
				size_t productionId;
				actionDefinitionStream >> commaDelim >> nonterminalId >> commaDelim >> productionId;
				Action *act = new Action('r', 0);
				act->setReduction(grammar->getReductionById(nonterminalId, productionId));
				action_table[stateNumber][terminal.second] = unique_ptr<Action> { act };
				continue;
			}
			case 'a':
				action_table[stateNumber][grammar->endSymbol] = unique_ptr<Action> { new Action('a', 0) };
				continue;
			case 'e':
				continue;
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
			string actionDefinition;
			table >> actionDefinition;
			istringstream actionDefinitionStream { actionDefinition };
			char type;
			actionDefinitionStream >> type;
			if (type == '0') {
				continue;
			}
			parse_state state;
			actionDefinitionStream >> state;
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
	ofstream table_out { "logs/parsing_table" };
	if (table_out.is_open()) {
		table_out << state_count << endl;
		table_out << "\%\%" << endl;
		for (int i = 0; i < state_count; i++) {
			for (auto& idToTerminal : idToTerminalMappingTable) {
				auto& act = action(i, idToTerminal.first);
				act.output(table_out);
			}
			table_out << endl;
		}
		table_out << "\%\%" << endl;
		for (int i = 0; i < state_count; i++) {
			for (auto& nonterminal : grammar->nonterminals) {
				try {
					int state = go_to(i, nonterminal);
					table_out << "g" << state << "\t";
				} catch (std::out_of_range&) {
					table_out << "00" << "\t";
				}
			}
			table_out << endl;
		}
		table_out.close();
	} else {
		throw std::runtime_error { "Unable to create parsing table output file!\n" };
	}
}

const Action& ParsingTable::action(unsigned state, unsigned terminalId) const {
	return *action_table[state].at(idToTerminalMappingTable.at(terminalId));
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
								if (action_table[state].find(expected.at(0)) == action_table[state].end()) {
									action_table[state][expected.at(0)] = unique_ptr<Action> { new Action('s', actionState) };
								} else {
									auto& conflict = *action_table[state].at(expected.at(0));
									switch (conflict.which()) {
									case 's':
										if (conflict.getState() == actionState)
											break;
										cerr << "\n!!!\n";
										cerr << "Shift/shift conflict in state " << state << endl;
										cerr << "Must be a BUG!!!\n";
										for (const auto& item : set) {
											cerr << item;
										}
										exit(1);
									case 'r':
										cerr << "\n!!!\n";
										cerr << "Shift/reduce conflict in state " << state << " on " << expected.at(0) << endl;
										for (const auto& item : set) {
											cerr << item;
										}
										exit(1);
									default:
										cerr << "\n!!!\n";
										cerr << "Unexpected conflict in state " << state << endl;
										for (const auto& item : set) {
											cerr << item;
										}
										exit(1);
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
					action_table[state][grammar->endSymbol] = unique_ptr<Action> { new Action('a', 0) };
				} else {
					for (size_t j = 0; j < item.getLookaheads().size(); j++) {
						if (action_table[state].find(item.getLookaheads().at(j)) == action_table[state].end()) {
							Action* action = new Action('r', 0);
							action->setReduction(item);
							action_table[state][item.getLookaheads().at(j)] = unique_ptr<Action> { action };
						} else {
							auto& conflict = *action_table[state].at(item.getLookaheads().at(j));
							switch (conflict.which()) {
							case 's':
								if (conflict.getState() == 0)
									break;
								cerr << "\n!!!\n";
								cerr << "Shift/reduce conflict in state " << state << " on " << item.getLookaheads().at(j) << endl;
								for (const auto& item : set) {
									cerr << item;
								}
								exit(1);
							case 'r':
								cerr << "\n!!!\n";
								cerr << "Reduce/reduce conflict in state " << state << endl;
								for (const auto& item : set) {
									cerr << item;
								}
								exit(1);
							default:
								cerr << "\n!!!\n";
								cerr << "Unexpected conflict in state " << state << endl;
								for (const auto& item : set) {
									cerr << item;
								}
								exit(1);
							}
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
	unsigned forge_token = 0;
	for (int state = 0; state < state_count; state++) {        // for each state
		unsigned term_size = 9999;
		unsigned term_id = 0;
		forge_token = 0;
		int errorState = 0;
		for (auto& idToTerminal : idToTerminalMappingTable) { // surandam galimą teisingą veiksmą
			try {
				auto& error_action = action(state, idToTerminal.first);
				errorState = error_action.getState();
				if (idToTerminal.second->getName().size() < term_size) {
					expected = idToTerminal.second;
					term_id = idToTerminal.first;
					term_size = idToTerminal.second->getName().size();
					if (error_action.which() == 'r') {
						forge_token = term_id;
					} else {
						forge_token = 0;
					}
				}
			} catch (std::out_of_range&) {

			}
		}
		if (term_id != 0) {
			try {
				auto& error_action = action(state, term_id);
				errorState = error_action.getState();
			} catch (std::out_of_range&) {
				errorState = 0;
			}
		}

		for (auto& idToTerminal : idToTerminalMappingTable) { // for each terminal
			try {
				action(state, idToTerminal.first);
			} catch (std::out_of_range&) {
				Action *err = new Action('e', errorState);
				err->setForge(forge_token);
				err->setExpected(expected);
				action_table[state][idToTerminal.second] = unique_ptr<Action> { err };
			}
		}
	}
}

std::shared_ptr<const GrammarSymbol> ParsingTable::getTerminalById(unsigned id) const {
	return idToTerminalMappingTable.at(id);
}
