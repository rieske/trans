#include "Grammar.h"

#include <limits.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <sstream>

#include "item.h"
#include "rule.h"

using std::cerr;
using std::endl;
using std::string;
using std::map;

using std::ifstream;

const string TERMINAL_CONFIG_DELIMITER = "\%\%";

string Grammar::start_symbol;
string Grammar::end_symbol;
vector<string> *Grammar::symbols;
vector<string> *Grammar::nonterminals;
map<unsigned, string> *Grammar::terminals;
map<string, vector<string> *> *Grammar::first_table;

Grammar::Grammar(const string bnfFileName) {
	ifstream bnfInputStream { bnfFileName };
	if (!bnfInputStream.is_open()) {
		throw std::invalid_argument("Unable to open bnf file for reading: " + bnfFileName);
	}
	readGrammarBnf(bnfInputStream);
	bnfInputStream.close();

	fillFirst();
	start_symbol = START_SYMBOL;
	end_symbol = END_SYMBOL;
	addTerminal((unsigned) (-1), end_symbol);
	addNonterminal(start_symbol);
}

Grammar::~Grammar() {
	delete symbols;
	symbols = NULL;
	delete nonterminals;
	nonterminals = NULL;
	delete terminals;
	terminals = NULL;
	delete first_table;
	first_table = NULL;
}

void Grammar::readGrammarBnf(ifstream& bnfInputStream) {
	string left;

	symbols = new vector<string>;
	nonterminals = new vector<string>;
	terminals = new map<unsigned, string>;

	string bnfToken;
	Rule *r = NULL;
	int ruleId = 1;
	while (bnfInputStream >> bnfToken) {
		if (bnfToken == TERMINAL_CONFIG_DELIMITER) {
			int terminalId;
			string terminal;
			while (bnfInputStream >> terminalId >> terminal) {
				addTerminal(terminalId, terminal);
			}
		} else if (bnfToken.length() == 1) {
			switch (bnfToken.at(0)) {
			case '|':
				rules.push_back(r);
				r = new Rule(left, ruleId++);
				break;
			case ';':
				rules.push_back(r);
				r = nullptr;
				break;
			case ':':
				break;
			default:
				throw std::runtime_error("Unrecognized control character in grammar configuration file: " + bnfToken);
			}
		} else if (!bnfToken.empty() && bnfToken.at(0) == '<' && bnfToken.at(bnfToken.length() - 1) == '>') {
			addNonterminal(bnfToken);
			if (r == nullptr) {
				for (auto nonterminal : *nonterminals) {
					if (nonterminal == bnfToken) {
						left = nonterminal;
						r = new Rule(left, ruleId++);
						break;
					}
				}
			} else {
				r->addRight(bnfToken);
			}
		} else if (!bnfToken.empty() && bnfToken.at(0) == '\'' && bnfToken.at(bnfToken.length() - 1) == '\'') {
			r->addRight(bnfToken);
		}
	}
	fillSymbols();
}

void Grammar::fillSymbols() {
	for (vector<string>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++)
		symbols->push_back(*it);
	for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++)
		symbols->push_back(it->second);
}

void Grammar::fillFirst() {
	first_table = new map<string, vector<string> *>;
	for (auto nonterminal : *nonterminals) {
		first_table->insert(make_pair(nonterminal, new vector<string>));
	}
	Grammar *ptr;
	bool more = false;

	do {
		more = false;
		for (unsigned j = 1; j < rules.size(); ++j) {
			Rule* rule = rules.at(j);
			vector<string> *right = rule->getRight();
			for (unsigned i = 0; i < right->size(); i++) {
				if (is_terminal(right->at(0))) {
					if (addFirst(rule->getLeft(), right->at(0)))     // jei tokio dar nebuvo
						more = true;
					break;
				} else if (is_nonterminal(right->at(0))) {
					if (addFirstRow(rule->getLeft(), right->at(0)))
						more = true;
					break;
				}
			}
		}
	} while (more);
}

bool Grammar::addFirst(string nonterm, string first) {
	if (!contains(first_table->at(nonterm), first)) {
		first_table->at(nonterm)->push_back(first);
		return true;
	}
	return false;
}

bool Grammar::addFirstRow(string dest, string src) {
	bool ret = false;
	for (vector<string>::const_iterator it = first_table->at(src)->begin(); it != first_table->at(src)->end(); it++) {
		if (addFirst(dest, *it))
			ret = true;
	}
	return ret;
}

void Grammar::print() const {
	for (Rule* rule : rules) {
		rule->print();
	}
	print_terminals();
	print_nonterminals();
	print_first_table();
}

void Grammar::printAddr() const {
	for (Rule* rule : rules) {
		rule->printAddr();
	}
}

void Grammar::print_terminals() const {
	cerr << "\nTerminals:\n";
	for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++) {
		cerr << it->second << ":" << it->first << endl;
	}
}

void Grammar::print_nonterminals() const {
	cerr << "\nNonterminals:\n";
	for (vector<string>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++) {
		cerr << *it << endl;
	}
}

void Grammar::print_first_table() const {
	cerr << "\nFirst table:\n";
	for (map<string, vector<string> *>::const_iterator it = first_table->begin(); it != first_table->end(); it++) {
		cerr << it->first << "\t:\t";
		for (vector<string>::const_iterator itf = it->second->begin(); itf != it->second->end(); itf++)
			cerr << *itf << " ";
		cerr << endl;
	}
}

void Grammar::output(ostream &out) const {
	for (Rule* rule : rules) {
		rule->log(out);
	}
	out << "\%\%" << endl;
	log_terminals(out);
	out << "\%\%" << endl;
}

void Grammar::log(ostream &out) const {
	for (Rule* rule : rules) {
		rule->log(out);
	}
	out << "\nTerminals:\n";
	log_terminals(out);
	out << "\nNonterminals:\n";
	log_nonterminals(out);
	out << "\nFirst table:\n";
	log_first_table(out);
}

void Grammar::log_terminals(ostream &out) const {
	for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++) {
		out << it->first << " " << it->second << endl;
	}
}

void Grammar::log_nonterminals(ostream &out) const {
	for (vector<string>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++) {
		out << *it << endl;
	}
}

void Grammar::log_first_table(ostream &out) const {
	for (map<string, vector<string> *>::const_iterator it = first_table->begin(); it != first_table->end(); it++) {
		out << it->first << "\t:\t";
		for (vector<string>::const_iterator itf = it->second->begin(); itf != it->second->end(); itf++)
			out << *itf << " ";
		out << endl;
	}
}

bool Grammar::contains(vector<string> *vect, string str) const {
	for (unsigned int i = 0; i < vect->size(); i++)
		if (vect->at(i) == str)
			return true;
	return false;
}

bool Grammar::is_terminal(string str) const {
	for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++) {
		if (it->second == str)
			return true;
	}
	return false;
}

bool Grammar::is_nonterminal(string str) const {
	for (vector<string>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++) {
		if (*it == str)
			return true;
	}
	return false;
}

Rule* Grammar::getRuleById(int ruleId) const {
	for (Rule* rule : rules) {
		if (rule->getId() == ruleId) {
			return rule;
		}
	}
	throw std::invalid_argument("Rule not found by id " + ruleId);
}

Rule* Grammar::getRuleByDefinition(const string& left, const vector<string>& right) const {
	for (Rule* rule : rules) {
		if (rule->getLeft() == left && *rule->getRight() == right) {
			return rule;
		}
	}
	throw std::invalid_argument("Rule not found by definition [" + left + "]");
}

const vector<string> *Grammar::getNonterminals() const {
	return nonterminals;
}

const map<unsigned, string> *Grammar::getTerminals() const {
	return terminals;
}

void Grammar::addTerminal(unsigned term_id, string term) {
	for (map<unsigned, string>::const_iterator it = terminals->begin(); it != terminals->end(); it++)
		if (it->second == term)
			return;
	terminals->insert(std::make_pair(term_id, term));
}

void Grammar::addNonterminal(string nonterm) {
	for (vector<string>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++)
		if (*it == nonterm)
			return;
	nonterminals->push_back(nonterm);
}

Set_of_items * Grammar::closure(Set_of_items * I) const {
	Set_of_items *i_ptr;
	bool more = false;
	vector<string> first_va_;

	do {
		more = false;
		i_ptr = I;

		while (i_ptr != NULL) {
			vector<string> *expected = i_ptr->getItem()->getExpected();
			if (expected->size() && is_nonterminal(expected->at(0)))     // [ A -> u.Bv, a ] (expected[0] == B)
					{
				first_va_.clear();
				if ((expected->size() > 1) && is_nonterminal(expected->at(1)))     // v - neterminalas
						{
					// XXX: kogero eis optimizuot
					for (vector<string>::const_iterator it = first_table->at(expected->at(1))->begin();
							it != first_table->at(expected->at(1))->end(); it++)
						first_va_.push_back(*it);
				} else if ((expected->size() > 1) && is_terminal(expected->at(1)))   // v - terminalas
					first_va_.push_back(expected->at(1));
				else {
					for (vector<string>::const_iterator it = i_ptr->getItem()->getLookaheads()->begin();
							it != i_ptr->getItem()->getLookaheads()->end(); it++)
						first_va_.push_back(*it);
				}

				for (Rule* rule : rules) {
					if (rule->getLeft() == expected->at(0))      // jei turim reikiamą taisyklę
							{
						for (vector<string>::const_iterator lookahead_iterator = first_va_.begin();
								lookahead_iterator != first_va_.end(); lookahead_iterator++) {
							Item *item = new Item(expected->at(0));
							item->setExpected(rule->getRight());
							item->addLookahead(*lookahead_iterator);
							if (I->addItem(item))
								more = true;
						}
					}
				}
			}
			i_ptr = i_ptr->getNext();
		}
	} while (more);
	return I;
}

Set_of_items *Grammar::go_to(Set_of_items *I, string X) const {
	Set_of_items *ret = NULL;

	while (I != NULL) {
		vector<string> *expected = I->getItem()->getExpected();
		if ((expected->size()) && (expected->at(0) == X))      // [ A -> a.Xb, c ]
				{
			Item *item = new Item(I->getItem()->getLeft());
			vector<string> *seen = I->getItem()->getSeen();
			for (unsigned i = 0; i < seen->size(); i++)
				item->addSeen(seen->at(i));
			item->addSeen(X);
			for (unsigned i = 1; i < expected->size(); i++)
				item->addExpected(expected->at(i));
			item->mergeLookaheads(I->getItem());

			if (ret == NULL)
				ret = new Set_of_items();
			ret->addItem(item);
		}
		I = I->getNext();
	}
	return closure(ret);
}

vector<Set_of_items *> *Grammar::canonical_collection() const {
	vector<Set_of_items *> *items = new vector<Set_of_items *>;
	Item *item = new Item(start_symbol);
	item->addExpected(rules.at(0)->getLeft());
	item->addLookahead(end_symbol);

	Set_of_items *initial_set = new Set_of_items();
	initial_set->addItem(item);
	initial_set = this->closure(initial_set);
	items->push_back(initial_set);

	for (unsigned i = 0; i < items->size(); i++) {        // for each set of items I in C
		for (unsigned j = 0; j < symbols->size(); j++) {  // and each grammar symbol X
			Set_of_items *tmp = go_to(items->at(i), symbols->at(j));
			if (tmp == NULL)  // such that goto(I, X) is not empty
				continue;
			bool was = false;
			for (unsigned k = 0; k < items->size(); k++) {                                           // and not in C
				if (*items->at(k) == *tmp) {
					was = true;
					break;
				}
			}
			if (was)
				delete tmp;
			else
				items->push_back(tmp);
		}
	}
	return items;
}
