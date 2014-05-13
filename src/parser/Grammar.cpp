#include "Grammar.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "FirstTable.h"
#include "GrammarRule.h"
#include "item.h"
#include "NonterminalSymbol.h"
#include "TerminalSymbol.h"

using std::cerr;
using std::endl;
using std::string;
using std::shared_ptr;
using std::unique_ptr;

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

	firstTable = unique_ptr<FirstTable> { new FirstTable { rules } };

	this->terminals.push_back(end_symbol);
	this->nonterminals.push_back(start_symbol);
}

Grammar::~Grammar() {
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
	/*for (auto it = firstTableDeprecated.begin(); it != firstTableDeprecated.end(); it++) {
	 out << it->first << "\t:\t";
	 for (auto itf = it->second.begin(); itf != it->second.end(); itf++)
	 out << *itf << " ";
	 out << endl;
	 }*/
}

shared_ptr<GrammarRule> Grammar::getRuleById(int ruleId) const {
	for (auto& rule : rules) {
		if (rule->getId() == ruleId) {
			return rule;
		}
	}
	throw std::invalid_argument("Rule not found by id " + ruleId);
}

shared_ptr<GrammarRule> Grammar::getRuleByDefinition(const shared_ptr<GrammarSymbol> nonterminal,
		const vector<shared_ptr<GrammarSymbol>>& production) const {
	for (auto& rule : rules) {
		if (rule->getNonterminal() == nonterminal && rule->getProduction() == production) {
			return rule;
		}
	}
	throw std::invalid_argument("Rule not found by definition [" + nonterminal->getName() + "]");
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

Set_of_items * Grammar::closure(Set_of_items * I) const {
	Set_of_items *i_ptr;
	bool more = false;
	vector<shared_ptr<GrammarSymbol>> first_va_;

	do {
		more = false;
		i_ptr = I;

		while (i_ptr != NULL) {
			vector<shared_ptr<GrammarSymbol>> expectedSymbols = i_ptr->getItem()->getExpected();
			if (!expectedSymbols.empty() && !expectedSymbols.at(0)->isTerminal()) {    // [ A -> u.Bv, a ] (expected[0] == B)
				first_va_.clear();
				if (expectedSymbols.size() > 1) {
					for (auto& va : firstTable->firstSet(expectedSymbols.at(1))) {
						first_va_.push_back(va);
					}
				} else {
					for (auto& lookahead : *i_ptr->getItem()->getLookaheads()) {
						first_va_.push_back(lookahead);
					}
				}

				for (auto& rule : rules) {
					if (rule->getNonterminal() == expectedSymbols.at(0)) {     // jei turim reikiamą taisyklę
						for (auto& lookahead : first_va_) {
							Item *item = new Item(expectedSymbols.at(0));
							item->setExpected(rule->getProduction());
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
		vector<shared_ptr<GrammarSymbol>> expectedSymbols = I->getItem()->getExpected();
		if ((!expectedSymbols.empty()) && (expectedSymbols.at(0) == X)) {      // [ A -> a.Xb, c ]
			Item *item = new Item(I->getItem()->getLeft());
			vector<std::shared_ptr<GrammarSymbol>> *seenSymbols = I->getItem()->getSeen();
			for (auto& seenSymbol : *seenSymbols) {
				item->addSeen(seenSymbol);
			}
			item->addSeen(X);
			for (auto expectedSymbolIterator = expectedSymbols.begin() + 1; expectedSymbolIterator != expectedSymbols.end();
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
	item->addExpected(rules.at(0)->getNonterminal());
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
