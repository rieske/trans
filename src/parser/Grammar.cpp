#include "Grammar.h"

#include <limits.h>
#include <cctype>
#include <cstdio>
#include <iterator>
#include <utility>

#include "item.h"

using std::cerr;
using std::endl;
using std::string;
using std::map;

string *Grammar::start_symbol;
string *Grammar::end_symbol;
vector<string *> *Grammar::symbols;
vector<string *> *Grammar::nonterminals;
map<unsigned, string *> *Grammar::terminals;
map<string *, vector<string *> *> *Grammar::first_table;

Grammar::Grammar(const char *bnf) {
	parseBnf(bnf);
	fillFirst();
	start_symbol = new string(START_SYMBOL);
	end_symbol = new string(END_SYMBOL);
	addTerminal((unsigned) (-1), end_symbol);
	addNonterminal(start_symbol);
}

Grammar::Grammar(Rule *r) {
	next = NULL;
	rule = r;
}

Grammar::~Grammar() {
	if (symbols != NULL)
		for (vector<string *>::iterator it = symbols->begin(); it != symbols->end(); it++) {
			delete *it;
			*it = NULL;
		}
	delete start_symbol;
	start_symbol = NULL;
	delete end_symbol;
	end_symbol = NULL;
	delete symbols;
	symbols = NULL;
	delete nonterminals;
	nonterminals = NULL;
	delete terminals;
	terminals = NULL;
	delete first_table;
	first_table = NULL;
	delete rule;
	rule = NULL;
	delete next;
}

void Grammar::parseBnf(const char *bnf_conf) {
	char *buffer = new char[MAX_CANON];
	FILE *bnf = NULL;
	next = NULL;
	rule = NULL;
	Rule *r = NULL;

	string token = "";
	string *left = NULL;

	if ( NULL == (bnf = fopen(bnf_conf, "r"))) {   // nepavyko atidaryti bnf gramatikos failo skaitymui
		cerr << "Error: could not open bnf file for reading. Filename: " << bnf_conf << endl;
		exit(1);
	}

	// susimetam visus simbolius į struktūras, vėliau žaisim su pointeriais, vardan efektyvumo
	symbols = new vector<string *>;
	nonterminals = new vector<string *>;
	terminals = new map<unsigned, string*>;

	while (NULL != (fgets(buffer, MAX_CANON, bnf))) {   // skaitom bnf failą
		if (strlen(buffer) >= 2) {
			if (0 == strncmp(buffer, "\%\%", 2))
				break;
		}
		string *nonterm = new string;
		for (unsigned int i = 0; i < strlen(buffer); i++) {
			if (isspace(buffer[i]) || buffer[i] != '<')     // kol ne neterminalo pradžia
				continue;
			for (; i < strlen(buffer); i++) {
				*nonterm += buffer[i];                 // kaupiam neterminalą
				if (buffer[i] == '>')
					break;
			}
			if (nonterm->at(nonterm->size() - 1) != '>')
				delete nonterm;
			else
				addNonterminal(nonterm);
			nonterm = new string;
		}
	}
	while (NULL != (fgets(buffer, MAX_CANON, bnf))) {
		int terminal_id;
		string *terminal_str = new string;
		bool started = false;
		if (0 != (terminal_id = atoi(buffer))) {
			for (unsigned i = 0; i < strlen(buffer); i++) {
				if (buffer[i] == '\'')
					started = !started;
				if (started)
					*terminal_str += buffer[i];
			}
			*terminal_str += '\'';
			addTerminal(terminal_id, terminal_str);
		}
	}
	fillSymbols();

	if ( NULL == (bnf = fopen(bnf_conf, "r"))) {
		cerr << "Error: could not open bnf file for reading. Filename: " << bnf_conf << endl;
		exit(1);
	}

	// o dabar renkam taisykles
	int ruleId = 1;
	while (NULL != (fgets(buffer, MAX_CANON, bnf))) {   // skaitom bnf failą
		if (strlen(buffer) >= 2) {
			if (0 == strncmp(buffer, "\%\%", 2))
				break;
		}
		for (unsigned int i = 0; i < strlen(buffer); i++) {
			if (!isspace(buffer[i])) {
				switch (buffer[i]) {
				case ':':
					break;
				case '|':
					if (buffer[i - 1] == '|' || buffer[i - 1] == '\'') {
						token += buffer[i];
						break;
					}
					addRule(r);
					r = new Rule(left, ruleId++);
					break;
				case ';':
					if (buffer[i - 1] == '\'') {
						token += buffer[i];
						break;
					}
					addRule(r);
					r = NULL;
					break;
				default:
					token += buffer[i];
				}
			} else if (token != "") // turim token'ą, žiūrim ką su juo daryti
					{
				if (r == NULL) {
					for (vector<string *>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++)
						if (**it == token) {
							left = *it;
							r = new Rule(left, ruleId++);
							break;
						}
				} else {
					for (vector<string *>::const_iterator it = symbols->begin(); it != symbols->end(); it++)
						if (**it == token) {
							r->addRight(*it);
							break;
						}
				}
				token = "";
			}
		}
	}
	delete[] buffer;
}

void Grammar::fillSymbols() {
	for (vector<string *>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++)
		symbols->push_back(*it);
	for (map<unsigned, string *>::const_iterator it = terminals->begin(); it != terminals->end(); it++)
		symbols->push_back(it->second);
}

void Grammar::fillFirst() {
	first_table = new map<string *, vector<string *> *>;
	for (vector<string *>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++)
		first_table->insert(make_pair(*it, new vector<string *>));

	Grammar *ptr;
	bool more = false;

	do {
		ptr = this->next;
		more = false;

		while (ptr) {
			vector<string *> *right = ptr->rule->getRight();

			for (unsigned i = 0; i < right->size(); i++) {
				if (is_terminal(right->at(0))) {
					if (addFirst(ptr->rule->getLeft(), right->at(0)))     // jei tokio dar nebuvo
						more = true;
					break;
				} else if (is_nonterminal(right->at(0))) {
					if (addFirstRow(ptr->rule->getLeft(), right->at(0)))
						more = true;
					break;
				}
			}

			ptr = ptr->next;
		}
	} while (more);
}

bool Grammar::addFirst(string *nonterm, string *first) {
	if (!contains(first_table->at(nonterm), first)) {
		first_table->at(nonterm)->push_back(first);
		return true;
	}
	return false;
}

bool Grammar::addFirstRow(string *dest, string *src) {
	bool ret = false;
	for (vector<string *>::const_iterator it = first_table->at(src)->begin(); it != first_table->at(src)->end(); it++) {
		if (addFirst(dest, *it))
			ret = true;
	}
	return ret;
}

void Grammar::print() const {
	if (rule != NULL)
		rule->print();
	if (next != NULL)
		next->print();
	else {
		print_terminals();
		print_nonterminals();
		print_first_table();
	}
}

void Grammar::printAddr() const {
	if (rule != NULL)
		rule->printAddr();
	if (next != NULL)
		next->printAddr();
}

void Grammar::print_terminals() const {
	cerr << "\nTerminals:\n";
	for (map<unsigned, string *>::const_iterator it = terminals->begin(); it != terminals->end(); it++) {
		cerr << *it->second << ":" << it->first << endl;
	}
}

void Grammar::print_nonterminals() const {
	cerr << "\nNonterminals:\n";
	for (vector<string *>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++) {
		cerr << **it << endl;
	}
}

void Grammar::print_first_table() const {
	cerr << "\nFirst table:\n";
	for (map<string *, vector<string *> *>::const_iterator it = first_table->begin(); it != first_table->end(); it++) {
		cerr << *it->first << "\t:\t";
		for (vector<string *>::const_iterator itf = it->second->begin(); itf != it->second->end(); itf++)
			cerr << **itf << " ";
		cerr << endl;
	}
}

void Grammar::output(ostream &out) const {
	if (rule != NULL)
		rule->log(out);
	if (next != NULL)
		next->output(out);
	else {
		out << "\%\%" << endl;
		log_terminals(out);
		out << "\%\%" << endl;
	}
}

void Grammar::log(ostream &out) const {
	if (rule != NULL)
		rule->log(out);
	if (next != NULL)
		next->log(out);
	else {
		out << "\nTerminals:\n";
		log_terminals(out);
		out << "\nNonterminals:\n";
		log_nonterminals(out);
		out << "\nFirst table:\n";
		log_first_table(out);
	}
}

void Grammar::log_terminals(ostream &out) const {
	for (map<unsigned, string *>::const_iterator it = terminals->begin(); it != terminals->end(); it++) {
		out << it->first << " " << *it->second << endl;
	}
}

void Grammar::log_nonterminals(ostream &out) const {
	for (vector<string *>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++) {
		out << **it << endl;
	}
}

void Grammar::log_first_table(ostream &out) const {
	for (map<string *, vector<string *> *>::const_iterator it = first_table->begin(); it != first_table->end(); it++) {
		out << *it->first << "\t:\t";
		for (vector<string *>::const_iterator itf = it->second->begin(); itf != it->second->end(); itf++)
			out << **itf << " ";
		out << endl;
	}
}

bool Grammar::contains(vector<string *> *vect, string *str) const {
	for (unsigned int i = 0; i < vect->size(); i++)
		if (vect->at(i) == str)
			return true;
	return false;
}

bool Grammar::is_terminal(string *str) const {
	for (map<unsigned, string *>::const_iterator it = terminals->begin(); it != terminals->end(); it++) {
		if (it->second == str)
			return true;
	}
	return false;
}

bool Grammar::is_nonterminal(string *str) const {
	for (vector<string *>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++) {
		if (*it == str)
			return true;
	}
	return false;
}

Rule *Grammar::getRule() const {
	if (rule != NULL)
		return rule;
	else if (next != NULL)
		return next->getRule();
	return NULL;
}

Grammar *Grammar::getNext() const {
	return next;
}

const vector<string *> *Grammar::getNonterminals() const {
	return nonterminals;
}

const map<unsigned, string *> *Grammar::getTerminals() const {
	return terminals;
}

void Grammar::addRule(Rule *r) {
	if (next != NULL)
		next->addRule(r);
	else
		next = new Grammar(r);
}

void Grammar::addTerminal(unsigned term_id, string *term) {
	for (map<unsigned, string *>::const_iterator it = terminals->begin(); it != terminals->end(); it++)
		if (*it->second == *term)
			return;
	terminals->insert(std::make_pair(term_id, term));
}

void Grammar::addNonterminal(string *nonterm) {
	for (vector<string *>::const_iterator it = nonterminals->begin(); it != nonterminals->end(); it++)
		if (**it == *nonterm)
			return;
	nonterminals->push_back(nonterm);
}

Set_of_items *Grammar::closure(Set_of_items *I) const {
	Set_of_items *i_ptr;
	Grammar *g_ptr;
	bool more = false;
	vector<string *> first_va_;

	do {
		more = false;
		i_ptr = I;

		while (i_ptr != NULL) {
			vector<string *> *expected = i_ptr->getItem()->getExpected();
			if (expected->size() && is_nonterminal(expected->at(0)))     // [ A -> u.Bv, a ] (expected[0] == B)
					{
				first_va_.clear();
				if ((expected->size() > 1) && is_nonterminal(expected->at(1)))     // v - neterminalas
						{
					// XXX: kogero eis optimizuot
					for (vector<string *>::const_iterator it = first_table->at(expected->at(1))->begin();
							it != first_table->at(expected->at(1))->end(); it++)
						first_va_.push_back(*it);
				} else if ((expected->size() > 1) && is_terminal(expected->at(1)))   // v - terminalas
					first_va_.push_back(expected->at(1));
				else {
					for (vector<string *>::const_iterator it = i_ptr->getItem()->getLookaheads()->begin();
							it != i_ptr->getItem()->getLookaheads()->end(); it++)
						first_va_.push_back(*it);
				}

				g_ptr = this->next;
				while (g_ptr != NULL)                   // einam per gramatikos taisykles
				{
					if (g_ptr->rule->getLeft() == expected->at(0))      // jei turim reikiamą taisyklę
							{
						for (vector<string *>::const_iterator lookahead_iterator = first_va_.begin();
								lookahead_iterator != first_va_.end(); lookahead_iterator++) {
							Item *item = new Item(expected->at(0));
							item->setExpected(g_ptr->rule->getRight());
							item->addLookahead(*lookahead_iterator);
							if (I->addItem(item))
								more = true;
						}
					}
					g_ptr = g_ptr->next;
				}
			}
			i_ptr = i_ptr->getNext();
		}
	} while (more);
	return I;
}

Set_of_items *Grammar::go_to(Set_of_items *I, string *X) const {
	Set_of_items *ret = NULL;

	while (I != NULL) {
		vector<string *> *expected = I->getItem()->getExpected();
		if ((expected->size()) && (expected->at(0) == X))      // [ A -> a.Xb, c ]
				{
			Item *item = new Item(I->getItem()->getLeft());
			vector<string *> *seen = I->getItem()->getSeen();
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
	item->addExpected(this->getRule()->getLeft());
	item->addLookahead(end_symbol);

	Set_of_items *initial_set = new Set_of_items();
	initial_set->addItem(item);
	initial_set = this->closure(initial_set);
	items->push_back(initial_set);

	for (unsigned i = 0; i < items->size(); i++)         // for each set of items I in C
			{
		for (unsigned j = 0; j < symbols->size(); j++)   // and each grammar symbol X
				{
			Set_of_items *tmp = go_to(items->at(i), symbols->at(j));
			if (tmp == NULL)                            // such that goto(I, X) is not empty
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
