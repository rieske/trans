#include "ParsingTable.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "BNFReader.h"
#include "Grammar.h"
#include "GrammarRule.h"
#include "NonterminalSymbol.h"
#include "TerminalSymbol.h"

using std::cerr;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;
using std::map;

ParsingTable::ParsingTable() {
	BNFReader bnfReader { "grammar.bnf" };

	grammar = new Grammar(bnfReader.getTerminals(), bnfReader.getNonterminals(), bnfReader.getRules());

	idToTerminalMappingTable = bnfReader.getIdToTerminalMappingTable();
	idToTerminalMappingTable[0] = grammar->getEndSymbol();

	terminals = grammar->getTerminals();
	nonterminals = grammar->getNonterminals();

	string cfgfile = "parsing_table";
	ifstream table_cfg { cfgfile };
	if (table_cfg.is_open()) {
		read_table(table_cfg);
	} else {
		cerr << "Error: could not open parsing table configuration file for reading. Filename: " << cfgfile << endl;
		exit(1);
	}
	table_cfg.close();
}

ParsingTable::ParsingTable(const string bnfFileName) {
	BNFReader bnfReader { bnfFileName };

	grammar = new Grammar(bnfReader.getTerminals(), bnfReader.getNonterminals(), bnfReader.getRules());

	idToTerminalMappingTable = bnfReader.getIdToTerminalMappingTable();
	idToTerminalMappingTable[0] = grammar->getEndSymbol();

	terminals = grammar->getTerminals();
	nonterminals = grammar->getNonterminals();

	items = grammar->canonical_collection();
	state_count = items.size();
	action_table = new map<std::shared_ptr<GrammarSymbol>, Action *> [state_count];
	goto_table = new map<std::shared_ptr<GrammarSymbol>, Action *> [state_count];

	fill_actions(items);
	fill_goto(items);
	fill_errors();
}

ParsingTable::~ParsingTable() {
	// trinam shift action'us
	for (map<long, Action *>::iterator it = shifts.begin(); it != shifts.end(); it++)
		delete it->second;
	// trinam reduce action'us
	for (unsigned i = 0; i < reductions.size(); i++) {
		delete reductions[i];
	}
	// trinam goto action'us
	for (map<long, Action *>::iterator it = gotos.begin(); it != gotos.end(); it++)
		delete it->second;
	// trinam error action'us
	for (map<long, Action *>::iterator it = errors.begin(); it != errors.end(); it++)
		delete it->second;
	delete[] action_table;
	delete[] goto_table;
	delete grammar;
}

void ParsingTable::read_table(ifstream &table) {
	table >> state_count;
	string delim;
	table >> delim;
	if (delim != "\%\%") {
		cerr << "Error in parsing table configuration file!\n";
		exit(1);
	}
	action_table = new map<std::shared_ptr<GrammarSymbol>, Action *> [state_count];
	goto_table = new map<std::shared_ptr<GrammarSymbol>, Action *> [state_count];

	string actionStr;

	// pildom action lentelę
	for (unsigned i = 0; i < state_count; i++) {       // for each state
		for (auto& idToTerminal : idToTerminalMappingTable) { // for each terminal
			Action *act = NULL;
			table >> actionStr;
			char type = actionStr[0];
			string stateStr = "";
			string reductionStr = "";
			string::const_iterator strIt = actionStr.begin() + 1;
			for (; strIt != actionStr.end(); strIt++) {
				if (!isdigit(*strIt))
					break;
				stateStr += *strIt;
			}
			long st = atoi(stateStr.c_str());
			unsigned reductionId;
			strIt++;
			switch (type) {
			case 's':
				try     // pabandom imt iš mapo pagal shiftinamą būseną
				{
					act = shifts.at(st);
				} catch (std::out_of_range)   // o jei napavyko, tai kuriam naują ir dedam į mapą
				{
					act = new Action('s', st);
					shifts.insert(std::make_pair(st, act));
				}
				action_table[i].insert(std::make_pair(idToTerminal.second, act));
				continue;
			case 'r':
				act = new Action('r', 0);
				for (; strIt != actionStr.end(); strIt++) {
					if (!isdigit(*strIt))
						break;
					reductionStr += *strIt;
				}
				reductionId = atoi(reductionStr.c_str());

				act->setReduction(grammar->getRuleById(reductionId));
				reductions.push_back(act);

				action_table[i].insert(std::make_pair(idToTerminal.second, act));
				continue;
			case 'a':
				act = new Action('a', 0);
				reductions.push_back(act);
				action_table[i].insert(std::make_pair(grammar->getEndSymbol(), act));
				continue;
			case 'e':
				continue;
			default:
				cerr << "Error in parsing table configuration file!\n";
				exit(1);
			}
		}
	}
	fill_errors();
	//print_actions();

	table >> delim;
	if (delim != "\%\%") {
		cerr << "Error in parsing table configuration file!\n";
		exit(1);
	}

	// pildom goto lentelę
	for (unsigned i = 0; i < state_count; i++) {       // for each state
		for (auto& nonterminal : nonterminals) {   // for each nonterminal
			Action *act = NULL;
			table >> actionStr;
			char type = actionStr[0];
			if (type == '0') {
				continue;
			}
			string stateStr = "";
			string::const_iterator strIt = actionStr.begin() + 1;
			for (; strIt != actionStr.end(); strIt++) {
				if (!isdigit(*strIt))
					break;
				stateStr += *strIt;
			}
			long st = atoi(stateStr.c_str());
			try {
				act = gotos.at(st);
			} catch (std::out_of_range) {
				act = new Action('g', st);
				gotos.insert(std::make_pair(st, act));
			}
			goto_table[i].insert(std::make_pair(nonterminal, act));
		}
	}
}

void ParsingTable::log(std::ostream &out) const {
	out << "Grammar:\n" << *grammar;   // <-- diagnostics

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

void ParsingTable::print_actions() const {
	cerr << "\nParsing table actions:\n\t";

	for (auto& idToTerminal : idToTerminalMappingTable) {
		cerr << idToTerminal.second << ":\t";
	}

	for (unsigned i = 0; i < state_count; i++) {
		cerr << endl << i << "\t";
		for (auto& idToTerminal : idToTerminalMappingTable) {
			Action *act = action(i, idToTerminal.first);
			if (act == NULL)
				cerr << "NULL\t";
			else
				act->print();
		}
	}
}

void ParsingTable::print_goto() const {
	cerr << "\nGoto transitions:\n\t";

	for (auto& nonterminal : nonterminals) {
		cerr << nonterminal << "\t";
	}

	for (unsigned i = 0; i < state_count; i++) {
		cerr << endl << i << "\t";
		for (auto& nonterminal : nonterminals) {
			Action *act = go_to(i, nonterminal);
			if (act == NULL)
				cerr << "NULL\t";
			else
				act->print();
		}
	}
}

void ParsingTable::output_html() const {
	ofstream html;
	const char *outfile = "logs/parsing_table.html";
	html.open(outfile);
	if (html.is_open()) {
		html << "<html>\n";
		html << "<body>\n";
		html << "<h2>Action table:</h2><br/>\n";
		html << "<table border=\"1\">\n";
		html << "<tr>\n";
		html << "<th>&nbsp;</th>";
		for (auto& idToTerminal : idToTerminalMappingTable)
			html << "<th>" << idToTerminal.second << "</th>";
		html << "\n</tr>\n";
		for (unsigned i = 0; i < state_count; i++) {
			html << "<tr>\n";
			html << "<th>" << i << "</th>";
			for (auto& idToTerminal : idToTerminalMappingTable) {
				Action *act = action(i, idToTerminal.first);
				if (act == NULL) {
					html << "<td>";
					html << "NULL";
					html << "</td>";
				} else {
					html << "<td align=\"center\">";
					if (('r' == act->which()) || ('s' == act->which()) || ('a' == act->which())) {
						html << "<b>";
						act->log(html);
						html << "</b>";
					} else
						act->log(html);
					html << "</td>";
				}
			}
			html << "\n</tr>";
		}
		html << "</table>\n";

		html << "<h2>Goto table:</h2><br/>\n";
		html << "<table border=\"1\">\n";
		html << "<tr>\n";
		html << "<th>&nbsp;</th>";
		for (auto& nonterminal : nonterminals) {
			html << "<th>" << nonterminal << "</th>";
		}
		html << "\n</tr>\n";
		for (unsigned i = 0; i < state_count; i++) {
			html << "<tr>\n";
			html << "<th>" << i << "</th>";
			for (auto& nonterminal : nonterminals) {
				html << "<td align=\"center\">";
				Action *act = go_to(i, nonterminal);
				if (act == NULL)
					html << "&nbsp;</td>";
				else
					act->log(html);
				html << "</td>";
			}
			html << "\n</tr>";
		}
		html << "</table>\n";
		html << "</body>\n";
		html << "</html>";
		html.close();
	} else
		cerr << "Unable to create html file! Filename: " << outfile << endl;
}

void ParsingTable::output_table() const {
	ofstream table_out;
	const char *outfile = "logs/parsing_table";
	table_out.open(outfile);
	if (table_out.is_open()) {
		table_out << state_count << endl;
		table_out << "\%\%" << endl;
		for (unsigned i = 0; i < state_count; i++) {
			for (auto& idToTerminal : idToTerminalMappingTable) {
				Action *act = action(i, idToTerminal.first);
				act->output(table_out);
			}
			table_out << endl;
		}
		table_out << "\%\%" << endl;
		for (unsigned i = 0; i < state_count; i++) {
			for (auto& nonterminal : nonterminals) {
				Action *act = go_to(i, nonterminal);
				if (act != NULL)
					act->output(table_out);
				else
					table_out << "00" << "\t";
			}
			table_out << endl;
		}
		table_out.close();
	} else
		cerr << "Unable to create parsing table output file! Filename: " << outfile << endl;
}

Action *ParsingTable::action(unsigned state, unsigned terminalId) const {
	if (state > state_count) {
		return NULL;
	}
	try {
		Action *action = action_table[state].at(idToTerminalMappingTable.at(terminalId));
		return action;
	} catch (std::out_of_range &err) {
		return NULL;
	}
}

Action *ParsingTable::go_to(unsigned state, std::shared_ptr<GrammarSymbol> nonterminal) const {
	if (state > state_count)
		return NULL;
	try {
		Action *action = goto_table[state].at(nonterminal);
		return action;
	} catch (std::out_of_range &err) {
		return NULL;
	}
}

int ParsingTable::fill_actions(vector<vector<LR1Item>> C) {
	for (unsigned long i = 0; i < state_count; i++) {    // for each state
		vector<LR1Item> set = C.at(i);
		for (const auto& item : set) {            // for each item in set
			vector<std::shared_ptr<GrammarSymbol>> expected = item.getExpected();
			Action *action = NULL;

			if (expected.size()) {
				if (expected.at(0)->isTerminal()) {
					vector<LR1Item> st = C.at(i);
					vector<LR1Item> gt = grammar->go_to(st, expected.at(0));
					if (!gt.empty()) {
						for (unsigned long j = 0; j < state_count; j++) {
							// XXX:
							if (C.at(j) == gt) {       // turim shift
								try {    // pabandom imt iš mapo pagal shiftinamą būseną
									action = shifts.at(j);
								} catch (std::out_of_range &) {   // o jei napavyko, tai kuriam naują ir dedam į mapą
									action = new Action('s', j);
									shifts.insert(std::make_pair(j, action));
								}
								try {
									Action *conflict = action_table[i].at(expected.at(0));
									switch (conflict->which()) {
									case 's':
										if (conflict->getState() == action->getState())
											break;
										cerr << "\n!!!\n";
										cerr << "Shift/shift conflict in state " << i << endl;
										cerr << "Must be a BUG!!!\n";
										for (const auto& item : set) {
											cerr << item;
										}
										exit(1);
									case 'r':
										cerr << "\n!!!\n";
										cerr << "Shift/reduce conflict in state " << i << " on " << expected.at(0) << endl;
										for (const auto& item : set) {
											cerr << item;
										}
										exit(1);
									default:
										cerr << "\n!!!\n";
										cerr << "Unexpected conflict in state " << i << endl;
										for (const auto& item : set) {
											cerr << item;
										}
										exit(1);
									}
								} catch (std::out_of_range &) {
									action_table[i].insert(std::make_pair(expected.at(0), action));
								}
								break;
							}
						}
					}
				}
			} else {     // dešinės pusės pabaiga
				if ((item.getDefiningSymbol() == grammar->getStartSymbol()) && (item.getLookaheads().at(0) == grammar->getEndSymbol())
						&& (expected.size() == 0)) {
					action = new Action('a', 0);
					reductions.push_back(action);
					action_table[i].insert(std::make_pair(grammar->getEndSymbol(), action));
				} else {
					action = new Action('r', 0);

					action->setReduction(grammar->getRuleByDefinition(item.getDefiningSymbol(), item.getVisited()));
					reductions.push_back(action);

					for (unsigned j = 0; j < item.getLookaheads().size(); j++) {
						try {
							Action *conflict = action_table[i].at(item.getLookaheads().at(j));
							switch (conflict->which()) {
							case 's':
								if (conflict->getState() == action->getState())
									break;
								cerr << "\n!!!\n";
								cerr << "Shift/reduce conflict in state " << i << " on " << item.getLookaheads().at(j) << endl;
								for (const auto& item : set) {
									cerr << item;
								}
								exit(1);
							case 'r':
								cerr << "\n!!!\n";
								cerr << "Reduce/reduce conflict in state " << i << endl;
								for (const auto& item : set) {
									cerr << item;
								}
								exit(1);
							default:
								cerr << "\n!!!\n";
								cerr << "Unexpected conflict in state " << i << endl;
								for (const auto& item : set) {
									cerr << item;
								}
								exit(1);
							}
						} catch (std::out_of_range &) {
						}
						action_table[i].insert(std::make_pair(item.getLookaheads().at(j), action));
					}
				}
			}
		}
	}
	return 0;
}

int ParsingTable::fill_goto(vector<vector<LR1Item>> C) {
	for (unsigned long i = 0; i < state_count; i++) {     // for each state
		vector<LR1Item> set = C.at(i);
		for (auto& nonterminal : nonterminals) {
			vector<LR1Item> gt = grammar->go_to(set, nonterminal);
			if (!gt.empty()) {
				for (unsigned long j = 0; j < state_count; j++) {
					// XXX:
					if (C.at(j) == gt) {
						Action *action;
						try {
							action = gotos.at(j);
						} catch (std::out_of_range) {
							action = new Action('g', j);
							gotos.insert(std::make_pair(j, action));
						}
						goto_table[i].insert(std::make_pair(nonterminal, action));
					}
				}
			}
		}
	}
	return 0;
}

void ParsingTable::fill_errors() {
	std::shared_ptr<GrammarSymbol> expected;
	unsigned forge_token = 0;
	for (unsigned long i = 0; i < state_count; i++)         // for each state
			{
		Action *error_action = NULL;

		unsigned term_size = 9999;
		unsigned term_id = 0;
		forge_token = 0;
		for (auto& idToTerminal : idToTerminalMappingTable) { // surandam galimą teisingą veiksmą
			error_action = action(i, idToTerminal.first);
			if ((error_action != NULL) && (idToTerminal.second->getName().size() < term_size)) {
				expected = idToTerminal.second;
				term_id = idToTerminal.first;
				term_size = idToTerminal.second->getName().size();
				if (error_action->which() == 'r')
					forge_token = term_id;
				else
					forge_token = 0;
			}
		}
		if (term_id != 0)
			error_action = action(i, term_id);
		if (error_action == NULL)
			error_action = new Action('e', 0);

		for (auto& idToTerminal : idToTerminalMappingTable) { // for each terminal
			Action *act = action(i, idToTerminal.first);
			if (act == NULL) {
				Action *err;
				try {
					err = errors.at(i);
				} catch (std::out_of_range) {
					err = new Action('e', error_action->getState());
					errors.insert(std::make_pair(i, err));
				}
				err->setForge(forge_token);
				err->setExpected(expected);
				action_table[i].insert(std::make_pair(idToTerminal.second, err));
			}
		}
	}
}

std::shared_ptr<GrammarSymbol> ParsingTable::getTerminalById(unsigned id) const {
	return idToTerminalMappingTable.at(id);
}
