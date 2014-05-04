#include "ParsingTable.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "Grammar.h"
#include "item.h"
#include "rule.h"

using std::cerr;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;
using std::map;

ParsingTable::ParsingTable() {
	grammar = new Grammar("grammar.bnf");
	terminals = grammar->getTerminals();
	nonterminals = grammar->getNonterminals();
	string cfgfile = "parsing_table";
	ifstream table_cfg { cfgfile };
	items = NULL;
	if (table_cfg.is_open()) {
		read_table(table_cfg);
	} else {
		cerr << "Error: could not open parsing table configuration file for reading. Filename: " << cfgfile << endl;
		exit(1);
	}
	table_cfg.close();
}

ParsingTable::ParsingTable(const string bnfFileName) {
	grammar = new Grammar(bnfFileName);
	terminals = grammar->getTerminals();
	nonterminals = grammar->getNonterminals();

	items = grammar->canonical_collection();
	state_count = items->size();
	action_table = new map<string, Action *> [state_count];
	goto_table = new map<string, Action *> [state_count];

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
		// hackas, kad apeit šitą nesąmonę:
		// (gdb) print *reductions[2]
		// $4 = {type = 96 '`', state = 0, reduction = 0x0, expected = 0x0}
		if (reductions[i]->which() != 'a' && reductions[i]->getReduction() != NULL)
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
	action_table = new map<string, Action *> [state_count];
	goto_table = new map<string, Action *> [state_count];

	string actionStr;

	for (unsigned i = 0; i < state_count; i++)      // pildom action lentelę
			{       // for each state
		for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++) { // for each terminal
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
				action_table[i].insert(std::make_pair(it->second, act));
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

				action_table[i].insert(std::make_pair(it->second, act));
				continue;
			case 'a':
				act = new Action('a', 0);
				reductions.push_back(act);
				action_table[i].insert(std::make_pair(END_SYMBOL, act));
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
		for (string nonterminal : *nonterminals) {   // for each nonterminal
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

void ParsingTable::log(ostream &out) const {
	out << "Grammar:\n";   // <-- diagnostics
	grammar->log(out);

	if (items != NULL) {
		out << "\n*********************";
		out << "\nCanonical collection:\n";
		out << "*********************\n";
		for (unsigned i = 0; i < items->size(); i++) {
			out << "Set " << i << ":\n";
			items->at(i)->log(out);
			out << endl;
		}
	}
}

void ParsingTable::print_actions() const {
	cerr << "\nParsing table actions:\n\t";

	for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++) {
		cerr << it->second << ":\t";
	}

	for (unsigned i = 0; i < state_count; i++) {
		cerr << endl << i << "\t";
		for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++) {
			Action *act = action(i, it->first);
			if (act == NULL)
				cerr << "NULL\t";
			else
				act->print();
		}
	}
}

void ParsingTable::print_goto() const {
	cerr << "\nGoto transitions:\n\t";

	for (string nonterminal : *nonterminals) {
		cerr << nonterminal << "\t";
	}

	for (unsigned i = 0; i < state_count; i++) {
		cerr << endl << i << "\t";
		for (auto nonterminal : *nonterminals) {
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
		for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++)
			html << "<th>" << it->second << "</th>";
		html << "\n</tr>\n";
		for (unsigned i = 0; i < state_count; i++) {
			html << "<tr>\n";
			html << "<th>" << i << "</th>";
			for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++) {
				Action *act = action(i, it->first);
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
		for (string nonterminal : *nonterminals) {
			html << "<th>" << nonterminal.substr(1, nonterminal.size() - 2) << "</th>";
		}
		html << "\n</tr>\n";
		for (unsigned i = 0; i < state_count; i++) {
			html << "<tr>\n";
			html << "<th>" << i << "</th>";
			for (string nonterminal : *nonterminals) {
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
			for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++) {
				Action *act = action(i, it->first);
				act->output(table_out);
			}
			table_out << endl;
		}
		table_out << "\%\%" << endl;
		for (unsigned i = 0; i < state_count; i++) {
			for (string nonterminal : *nonterminals) {
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

bool ParsingTable::v_are_equal(vector<string> v1, vector<string> v2) const {
	if (v1.size() != v2.size())
		return false;
	for (unsigned i = 0; i < v1.size(); i++)
		if (v1[i] != v2[i])
			return false;
	return true;
}

Action *ParsingTable::action(unsigned state, unsigned terminal) const {
	if (state > state_count) {
		return NULL;
	}
	try {
		Action *action = action_table[state].at(terminals->at(terminal));
		return action;
	} catch (std::out_of_range &err) {
		return NULL;
	}
}

Action *ParsingTable::go_to(unsigned state, string nonterminal) const {
	if (state > state_count)
		return NULL;
	try {
		Action *action = goto_table[state].at(nonterminal);
		return action;
	} catch (std::out_of_range &err) {
		return NULL;
	}
}

int ParsingTable::fill_actions(vector<Set_of_items *> *C) {
	for (unsigned long i = 0; i < state_count; i++)     // for each state
			{
		Set_of_items *set = (*C)[i];
		while (set != NULL)             // for each item in set
		{
			Item *item = set->getItem();
			vector<string> *expected = item->getExpected();
			Action *action = NULL;

			if (expected->size()) {
				if (grammar->is_terminal(expected->at(0))) {
					Set_of_items *st = (*C)[i];
					Set_of_items *gt = grammar->go_to(st, expected->at(0));
					if (gt != NULL) {
						for (unsigned long j = 0; j < state_count; j++) {
							if (*(*C)[j] == (*gt))        // turim shift
									{
								try     // pabandom imt iš mapo pagal shiftinamą būseną
								{
									action = shifts.at(j);
								} catch (std::out_of_range &)   // o jei napavyko, tai kuriam naują ir dedam į mapą
								{
									action = new Action('s', j);
									shifts.insert(std::make_pair(j, action));
								}
								try {
									Action *conflict = action_table[i].at(expected->at(0));
									switch (conflict->which()) {
									case 's':
										if (conflict->getState() == action->getState())
											break;
										cerr << "\n!!!\n";
										cerr << "Shift/shift conflict in state " << i << endl;
										cerr << "Must be a BUG!!!\n";
										(*C)[i]->print();
										exit(1);
									case 'r':
										cerr << "\n!!!\n";
										cerr << "Shift/reduce conflict in state " << i << " on " << expected->at(0)
												<< endl;
										(*C)[i]->print();
										exit(1);
									default:
										cerr << "\n!!!\n";
										cerr << "Unexpected conflict in state " << i << endl;
										(*C)[i]->print();
										exit(1);
									}
								} catch (std::out_of_range &) {
									action_table[i].insert(std::make_pair(expected->at(0), action));
								}
								break;
							}
						}
					}
				}
			} else      // dešinės pusės pabaiga
			{
				if ((item->getLeft() == START_SYMBOL) && (item->getLookaheads()->at(0) == END_SYMBOL)
						&& (expected->size() == 0)) {
					action = new Action('a', 0);
					reductions.push_back(action);
					action_table[i].insert(std::make_pair(END_SYMBOL, action));
				} else {
					action = new Action('r', 0);

					action->setReduction(grammar->getRuleByDefinition(item->getLeft(), *item->getSeen()));
					reductions.push_back(action);

					for (unsigned j = 0; j < item->getLookaheads()->size(); j++) {
						try {
							Action *conflict = action_table[i].at(item->getLookaheads()->at(j));
							switch (conflict->which()) {
							case 's':
								if (conflict->getState() == action->getState())
									break;
								cerr << "\n!!!\n";
								cerr << "Shift/reduce conflict in state " << i << " on " << item->getLookaheads()->at(j)
										<< endl;
								(*C)[i]->print();
								exit(1);
							case 'r':
								cerr << "\n!!!\n";
								cerr << "Reduce/reduce conflict in state " << i << endl;
								(*C)[i]->print();
								exit(1);
							default:
								cerr << "\n!!!\n";
								cerr << "Unexpected conflict in state " << i << endl;
								(*C)[i]->print();
								exit(1);
							}
						} catch (std::out_of_range &) {
						}
						action_table[i].insert(std::make_pair(item->getLookaheads()->at(j), action));
					}
				}
			}
			set = set->getNext();
		}
	}
	return 0;
}

int ParsingTable::fill_goto(vector<Set_of_items *> *C) {
	for (unsigned long i = 0; i < state_count; i++){     // for each state
		Set_of_items *set = (*C)[i];
		for (string nonterminal : *nonterminals) {
			Set_of_items *gt = grammar->go_to(set, nonterminal);
			if (gt != NULL) {
				for (unsigned long j = 0; j < state_count; j++) {
					if (*(*C)[j] == (*gt)) {
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
	string expected = "";
	unsigned forge_token = 0;
	for (unsigned long i = 0; i < state_count; i++)         // for each state
			{
		Action *error_action = NULL;

		unsigned term_size = 9999;
		unsigned term_id = 0;
		forge_token = 0;
		for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++) // surandam galimą teisingą veiksmą
				{
			error_action = action(i, it->first);
			if ((error_action != NULL) && (it->second.size() < term_size)) {
				expected = it->second;
				term_id = it->first;
				term_size = it->second.size();
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

		for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++) // for each terminal
				{
			Action *act = action(i, it->first);
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
				action_table[i].insert(std::make_pair(it->second, err));
			}
		}
	}
}

string ParsingTable::getTerminalById(unsigned id) const {
	return terminals->at(id);
}
