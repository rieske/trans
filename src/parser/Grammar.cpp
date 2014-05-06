#include "Grammar.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "item.h"

using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::set;

using std::ifstream;

const string TERMINAL_CONFIG_DELIMITER = "\%\%";

Grammar::Grammar(const string bnfFileName) {
	ifstream bnfInputStream { bnfFileName };
	if (!bnfInputStream.is_open()) {
		throw std::invalid_argument("Unable to open bnf file for reading: " + bnfFileName);
	}
	readGrammarBnf(bnfInputStream);
	bnfInputStream.close();

	computeFirstSets();
	start_symbol = START_SYMBOL;
	end_symbol = END_SYMBOL;
	terminals[0] = end_symbol;
	addNonterminal(start_symbol);
}

Grammar::~Grammar() {
}

void Grammar::readGrammarBnf(ifstream& bnfInputStream) {
	string bnfToken;
	Rule *rule { nullptr };
	string left;
	int ruleId { 1 };
	while (bnfInputStream >> bnfToken) {
		if (bnfToken == TERMINAL_CONFIG_DELIMITER) {
			int terminalId;
			string terminal;
			while (bnfInputStream >> terminalId >> terminal) {
				terminals[terminalId] = terminal;
			}
		} else if (bnfToken.length() == 1) {
			switch (bnfToken.at(0)) {
			case '|':
				rules.push_back(rule);
				rule = new Rule(left, ruleId++);
				break;
			case ';':
				rules.push_back(rule);
				rule = nullptr;
				break;
			case ':':
				break;
			default:
				throw std::runtime_error("Unrecognized control character in grammar configuration file: " + bnfToken);
			}
		} else if (!bnfToken.empty() && bnfToken.at(0) == '<' && bnfToken.at(bnfToken.length() - 1) == '>') {
			addNonterminal(bnfToken);
			if (rule) {
				rule->addRight(bnfToken);
			} else {
				left = bnfToken;
				rule = new Rule(left, ruleId++);
			}
		} else if (!bnfToken.empty() && *bnfToken.begin() == '\'' && *(bnfToken.end() - 1) == '\'') {
			rule->addRight(bnfToken);
		}
	}
	for (auto& idToTerminal : terminals) {
		symbols.insert(idToTerminal.second);
	}
	symbols.insert(nonterminals.begin(), nonterminals.end());
}

void Grammar::computeFirstSets() {
	for (auto& nonterminal : nonterminals) {
		nonterminalFirstSets[nonterminal] = set<string> { };
	}
	bool more = false;

	do {
		more = false;
		for (unsigned j = 1; j < rules.size(); ++j) {
			Rule* rule = rules.at(j);
			vector<string> *right = rule->getRight();
			for (unsigned i = 0; i < right->size(); i++) {
				string firstSymbol = right->at(0);
				if (is_terminal(firstSymbol)) {
					if (addFirst(rule->getLeft(), firstSymbol))     // jei tokio dar nebuvo
						more = true;
					break;
				} else if (is_nonterminal(firstSymbol)) {
					if (addFirstRow(rule->getLeft(), firstSymbol))
						more = true;
					break;
				}
			}
		}
	} while (more);
}

bool Grammar::addFirst(string nonterm, string first) {
	set<string>& firstForNonterminal = nonterminalFirstSets.at(nonterm);
	if (std::find(firstForNonterminal.begin(), firstForNonterminal.end(), first) == firstForNonterminal.end()) {
		firstForNonterminal.insert(first);
		return true;
	}
	return false;
}

bool Grammar::addFirstRow(string dest, string src) {
	bool ret = false;
	for (auto& firstSymbol : nonterminalFirstSets.at(src)) {
		if (addFirst(dest, firstSymbol))
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

void Grammar::print_terminals() const {
	cerr << "\nTerminals:\n";
	for (auto& idToTerminal : terminals) {
		cerr << idToTerminal.second << ":" << idToTerminal.first << endl;
	}
}

void Grammar::print_nonterminals() const {
	cerr << "\nNonterminals:\n";
	for (auto& nonterminal : nonterminals) {
		cerr << nonterminal << endl;
	}
}

void Grammar::print_first_table() const {
	cerr << "\nFirst table:\n";
	for (auto it = nonterminalFirstSets.begin(); it != nonterminalFirstSets.end(); it++) {
		cerr << it->first << "\t:\t";
		for (auto itf = it->second.begin(); itf != it->second.end(); itf++)
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
	for (auto& idToTerminal : terminals) {
		out << idToTerminal.first << " " << idToTerminal.second << endl;
	}
}

void Grammar::log_nonterminals(ostream &out) const {
	for (auto& nonterminal : nonterminals) {
		out << nonterminal << endl;
	}
}

void Grammar::log_first_table(ostream &out) const {
	for (auto it = nonterminalFirstSets.begin(); it != nonterminalFirstSets.end(); it++) {
		out << it->first << "\t:\t";
		for (auto itf = it->second.begin(); itf != it->second.end(); itf++)
			out << *itf << " ";
		out << endl;
	}
}

bool Grammar::is_terminal(string str) const {
	for (auto& idToTerminal : terminals) {
		if (idToTerminal.second == str)
			return true;
	}
	return false;
}

bool Grammar::is_nonterminal(string str) const {
	for (auto& nonterminal : nonterminals) {
		if (nonterminal == str)
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

set<string> Grammar::getNonterminals() const {
	return nonterminals;
}

map<unsigned, string> Grammar::getTerminals() const {
	return terminals;
}

void Grammar::addNonterminal(string nonterm) {
	nonterminals.insert(nonterm);
}

Set_of_items* Grammar::closure(Set_of_items * I) const {
	Set_of_items *i_ptr;
	bool more = false;
	vector<string> first_va_;

	do {
		more = false;
		i_ptr = I;

		while (i_ptr != NULL) {
			vector<string> *expected = i_ptr->getItem()->getExpected();
			if (!expected->empty() && is_nonterminal(expected->at(0))) {    // [ A -> u.Bv, a ] (expected[0] == B)
				first_va_.clear();
				if ((expected->size() > 1) && is_nonterminal(expected->at(1))) {    // v - neterminalas
				// XXX: kogero eis optimizuot
					for (auto& va : nonterminalFirstSets.at(expected->at(1))) {
						first_va_.push_back(va);
					}
				} else if ((expected->size() > 1) && is_terminal(expected->at(1))) {  // v - terminalas
					first_va_.push_back(expected->at(1));
				} else {
					for (auto& lookahead : *i_ptr->getItem()->getLookaheads()) {
						first_va_.push_back(lookahead);
					}
				}

				for (Rule* rule : rules) {
					if (rule->getLeft() == expected->at(0)) {     // jei turim reikiamą taisyklę
						for (auto& lookahead : first_va_) {
							Item *item = new Item(expected->at(0));
							item->setExpected(rule->getRight());
							item->addLookahead(lookahead);
							if (I->addItem(item)) {
								more = true;
							}
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
		vector<string> *expectedSymbols = I->getItem()->getExpected();
		if ((!expectedSymbols->empty()) && (expectedSymbols->at(0) == X)) {      // [ A -> a.Xb, c ]
			Item *item = new Item(I->getItem()->getLeft());
			vector<string> *seenSymbols = I->getItem()->getSeen();
			for (auto& seenSymbol : *seenSymbols) {
				item->addSeen(seenSymbol);
			}
			item->addSeen(X);
			for (auto expectedSymbolIterator = expectedSymbols->begin() + 1; expectedSymbolIterator != expectedSymbols->end();
					++expectedSymbolIterator) {
				item->addExpected(*expectedSymbolIterator);
			}
			item->mergeLookaheads(I->getItem());

			if (ret == NULL) {
				ret = new Set_of_items();
			}
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
		for (auto& symbol : symbols) {  // and each grammar symbol X
			Set_of_items *tmp = go_to(items->at(i), symbol);
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
