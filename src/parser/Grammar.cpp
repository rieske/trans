#include "Grammar.h"

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>

#include "FirstTable.h"
#include "GrammarRule.h"
#include "NonterminalSymbol.h"
#include "TerminalSymbol.h"

using std::shared_ptr;
using std::unique_ptr;
using std::vector;

Grammar::Grammar(const vector<shared_ptr<GrammarSymbol>> terminals,
		const vector<shared_ptr<GrammarSymbol>> nonterminals, const vector<shared_ptr<GrammarRule>> rules) :
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

vector<shared_ptr<GrammarSymbol>> Grammar::getNonterminals() const {
	return nonterminals;
}

vector<std::shared_ptr<GrammarSymbol>> Grammar::getTerminals() const {
	return terminals;
}

shared_ptr<GrammarSymbol> Grammar::getStartSymbol() const {
	return start_symbol;
}

shared_ptr<GrammarSymbol> Grammar::getEndSymbol() const {
	return end_symbol;
}

vector<Item> Grammar::closure(vector<Item> I) const {
	bool more = false;
	vector<shared_ptr<GrammarSymbol>> first_va_;

	do {
		more = false;
		for (size_t i = 0; i < I.size(); ++i) {
			const Item& item = I.at(i);
			vector<shared_ptr<GrammarSymbol>> expectedSymbols = item.getExpected();
			if (!expectedSymbols.empty() && !expectedSymbols.at(0)->isTerminal()) { // [ A -> u.Bv, a ] (expected[0] == B)
				first_va_.clear();
				if (expectedSymbols.size() > 1) {
					for (auto& va : firstTable->firstSet(expectedSymbols.at(1))) {
						first_va_.push_back(va);
					}
				} else {
					for (auto& lookahead : item.getLookaheads()) {
						first_va_.push_back(lookahead);
					}
				}

				for (auto& rule : rules) {
					if (rule->getNonterminal() == expectedSymbols.at(0)) {     // jei turim reikiamą taisyklę
						for (auto& lookahead : first_va_) {
							Item item { expectedSymbols.at(0) };
							item.setExpected(rule->getProduction());
							item.addLookahead(lookahead);
							bool add = true;
							for (auto& itm : I) {
								if (itm.coresAreEqual(item)) {
									itm.mergeLookaheads(item);
									add = false;
									break;
								}
							}
							if (add) {
								I.push_back(item);
								more = true;
							}
						}
					}
				}
			}
		}
	} while (more);
	return I;
}

vector<Item> Grammar::go_to(vector<Item> I, const shared_ptr<GrammarSymbol> X) const {
	vector<Item> ret;
	for (const auto& existingItem : I) {
		vector<shared_ptr<GrammarSymbol>> expectedSymbols = existingItem.getExpected();
		if ((!expectedSymbols.empty()) && (expectedSymbols.at(0) == X)) {      // [ A -> a.Xb, c ]
			Item item { existingItem.getLeft() };
			vector<shared_ptr<GrammarSymbol>> seenSymbols = existingItem.getSeen();
			for (auto& seenSymbol : seenSymbols) {
				item.addSeen(seenSymbol);
			}
			item.addSeen(X);
			for (auto expectedSymbolIterator = expectedSymbols.begin() + 1;
					expectedSymbolIterator != expectedSymbols.end(); ++expectedSymbolIterator) {
				item.addExpected(*expectedSymbolIterator);
			}

			item.mergeLookaheads(existingItem);

			bool add = true;
			for (auto& itm : ret) {
				if (itm.coresAreEqual(item)) {
					itm.mergeLookaheads(item);
					add = false;
					break;
				}
			}
			if (add) {
				ret.push_back(item);
			}
		}
	}

	return closure(ret);
}

vector<vector<Item>> Grammar::canonical_collection() const {
	vector<vector<Item>> collection;
	Item item { start_symbol };
	item.addExpected(rules.at(0)->getNonterminal());
	item.addLookahead(end_symbol);

	vector<Item> initial_set;
	initial_set.push_back(item);
	initial_set = this->closure(initial_set);
	collection.push_back(initial_set);

	for (unsigned i = 0; i < collection.size(); i++) {        // for each set of items I in C
		for (auto& symbol : symbols) {  // and each grammar symbol X
			vector<Item> tmp = go_to(collection.at(i), symbol);
			if (tmp.empty())  // such that goto(I, X) is not empty
				continue;
			bool was = false;
			for (const auto& set : collection) { // and not in C
				if (set == tmp) {
					was = true;
					break;
				}
			}
			if (!was) {
				collection.push_back(tmp);
			}
		}
	}
	return collection;
}

std::ostream& operator<<(std::ostream& out, const Grammar& grammar) {
	for (auto& rule : grammar.rules) {
		out << rule;
	}
	out << "\nTerminals:\n";
	for (auto& terminal : grammar.terminals) {
		out << terminal << "\n";
	}
	out << "\nNonterminals:\n";
	for (auto& nonterminal : grammar.nonterminals) {
		out << nonterminal << "\n";
	}
	out << "\nFirst table:\n";
	out << *grammar.firstTable;
	return out;
}
