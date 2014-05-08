#include "Grammar.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "item.h"
#include "NonterminalSymbol.h"
#include "TerminalSymbol.h"

using std::cerr;
using std::endl;
using std::string;
using std::shared_ptr;

using std::ifstream;

Grammar::Grammar(const std::vector<std::shared_ptr<GrammarSymbol>> terminals,
		const std::vector<std::shared_ptr<GrammarSymbol>> nonterminals, const std::vector<std::shared_ptr<GrammarRule>> rules) :
		start_symbol { shared_ptr<GrammarSymbol> { new NonterminalSymbol { "<__start__>" } } },
		end_symbol { shared_ptr<GrammarSymbol> { new TerminalSymbol { "'$end$'" } } } {
	this->terminals = terminals;
	this->nonterminals = nonterminals;
	this->rules = rules;
	symbols.insert(symbols.begin(), this->terminals.begin(), this->terminals.end());
	symbols.insert(symbols.begin(), this->nonterminals.begin(), this->nonterminals.end());
	computeFirstSets();
	this->terminals.push_back(end_symbol);
	this->nonterminals.push_back(start_symbol);
}

Grammar::~Grammar() {
}

void Grammar::computeFirstSets() {
	for (auto& nonterminal : nonterminals) {
		nonterminalFirstSets[nonterminal] = vector<shared_ptr<GrammarSymbol>> { };
	}
	bool more = false;

	do {
		more = false;
		for (unsigned j = 1; j < rules.size(); ++j) {
			auto& rule = rules.at(j);
			vector<shared_ptr<GrammarSymbol>> *right = rule->getRight();
			for (unsigned i = 0; i < right->size(); ++i) {
				shared_ptr<GrammarSymbol> firstSymbol = right->at(0);
				if (firstSymbol->isTerminal()) {
					if (addFirst(rule->getLeft(), firstSymbol))     // jei tokio dar nebuvo
						more = true;
					break;
				} else {
					if (addFirstRow(rule->getLeft(), firstSymbol))
						more = true;
					break;
				}
			}
		}
	} while (more);
}

bool Grammar::addFirst(shared_ptr<GrammarSymbol> nonterm, shared_ptr<GrammarSymbol> first) {
	auto& firstForNonterminal = nonterminalFirstSets.at(nonterm);
	if (std::find(firstForNonterminal.begin(), firstForNonterminal.end(), first) == firstForNonterminal.end()) {
		firstForNonterminal.push_back(first);
		return true;
	}
	return false;
}

bool Grammar::addFirstRow(shared_ptr<GrammarSymbol> dest, shared_ptr<GrammarSymbol> src) {
	bool ret = false;
	for (auto& firstSymbol : nonterminalFirstSets.at(src)) {
		if (addFirst(dest, firstSymbol))
			ret = true;
	}
	return ret;
}

void Grammar::print() const {
	for (auto& rule : rules) {
		rule->print();
	}
	print_terminals();
	print_nonterminals();
	print_first_table();
}

void Grammar::print_terminals() const {
	cerr << "\nTerminals:\n";
	for (auto& terminal : terminals) {
		cerr << terminal << endl;
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
	for (auto& rule : rules) {
		rule->log(out);
	}
	out << "\%\%" << endl;
	log_terminals(out);
	out << "\%\%" << endl;
}

void Grammar::log(ostream &out) const {
	for (auto& rule : rules) {
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
	for (auto& terminal : terminals) {
		out << terminal << endl;
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

shared_ptr<GrammarRule> Grammar::getRuleById(int ruleId) const {
	for (auto& rule : rules) {
		if (rule->getId() == ruleId) {
			return rule;
		}
	}
	throw std::invalid_argument("Rule not found by id " + ruleId);
}

shared_ptr<GrammarRule> Grammar::getRuleByDefinition(const shared_ptr<GrammarSymbol> left, const vector<shared_ptr<GrammarSymbol>>& right) const {
	for (auto& rule : rules) {
		if (rule->getLeft() == left && *rule->getRight() == right) {
			return rule;
		}
	}
	throw std::invalid_argument("Rule not found by definition [" + left->getName() + "]");
}

vector<std::shared_ptr<GrammarSymbol>> Grammar::getNonterminals() const {
	return nonterminals;
}

vector<std::shared_ptr<GrammarSymbol>> Grammar::getTerminals() const {
	return terminals;
}

std::shared_ptr<GrammarSymbol> Grammar::getStartSymbol() const {
	return start_symbol;
}

std::shared_ptr<GrammarSymbol> Grammar::getEndSymbol() const {
	return end_symbol;
}

Set_of_items* Grammar::closure(Set_of_items * I) const {
	Set_of_items *i_ptr;
	bool more = false;
	vector<shared_ptr<GrammarSymbol>> first_va_;

	do {
		more = false;
		i_ptr = I;

		while (i_ptr != NULL) {
			vector<shared_ptr<GrammarSymbol>> *expected = i_ptr->getItem()->getExpected();
			if (!expected->empty() && !expected->at(0)->isTerminal()) {    // [ A -> u.Bv, a ] (expected[0] == B)
				first_va_.clear();
				if ((expected->size() > 1) && !expected->at(1)->isTerminal()) {    // v - neterminalas
						// XXX: kogero eis optimizuot
					for (auto& va : nonterminalFirstSets.at(expected->at(1))) {
						first_va_.push_back(va);
					}
				} else if ((expected->size() > 1) && expected->at(1)->isTerminal()) {  // v - terminalas
					first_va_.push_back(expected->at(1));
				} else {
					for (auto& lookahead : *i_ptr->getItem()->getLookaheads()) {
						first_va_.push_back(lookahead);
					}
				}

				for (auto& rule : rules) {
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

Set_of_items *Grammar::go_to(Set_of_items *I, const std::shared_ptr<GrammarSymbol> X) const {
	Set_of_items *ret = NULL;

	while (I != NULL) {
		vector<shared_ptr<GrammarSymbol>> *expectedSymbols = I->getItem()->getExpected();
		if ((!expectedSymbols->empty()) && (expectedSymbols->at(0) == X)) {      // [ A -> a.Xb, c ]
			Item *item = new Item(I->getItem()->getLeft());
			vector<std::shared_ptr<GrammarSymbol>> *seenSymbols = I->getItem()->getSeen();
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
