#include "Grammar.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>

#include "FirstTable.h"
#include "GrammarRule.h"
#include "GrammarRuleBuilder.h"
#include "NonterminalSymbol.h"
#include "TerminalSymbol.h"

using std::shared_ptr;
using std::unique_ptr;
using std::vector;

Grammar::Grammar(const vector<shared_ptr<GrammarSymbol>> terminals, const vector<shared_ptr<GrammarSymbol>> nonterminals,
		const vector<shared_ptr<GrammarRule>> rules) :
		start_symbol { std::make_shared<NonterminalSymbol>("<__start__>") },
		end_symbol { std::make_shared<TerminalSymbol>("'$end$'") } {
	this->terminals = terminals;
	this->nonterminals = nonterminals;
	this->rules = rules;
	this->terminals.push_back(end_symbol);
	this->nonterminals.push_back(start_symbol);

	firstTable = unique_ptr<FirstTable> { new FirstTable { rules } };
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

vector<LR1Item> Grammar::closure(vector<LR1Item> I) const {

	bool more = true;
	while (more) {
		more = false;
		for (size_t i = 0; i < I.size(); ++i) {
			const LR1Item& item = I.at(i);
			const vector<shared_ptr<GrammarSymbol>>& expectedSymbols = item.getExpected();
			if (!expectedSymbols.empty() && !expectedSymbols.at(0)->isTerminal()) { // [ A -> u.Bv, a ] (expected[0] == B)
				vector<shared_ptr<GrammarSymbol>> firstForNextSymbol;
				if (expectedSymbols.size() > 1) {
					firstForNextSymbol = firstTable->firstSet(expectedSymbols.at(1));
				} else {
					firstForNextSymbol = item.getLookaheads();
				}
				for (const auto& rule : rules) {
					if (rule->getNonterminal() == expectedSymbols.at(0)) {
						LR1Item newItem { rule, firstForNextSymbol };
						const auto& existingItemIt = std::find_if(I.begin(), I.end(),
								[&newItem] (const LR1Item& existingItem) {return existingItem.coresAreEqual(newItem);});
						if (existingItemIt == I.end()) {
							I.push_back(newItem);
							more = true;
						} else {
							existingItemIt->mergeLookaheads(newItem.getLookaheads());
						}
					}
				}
			}
		}
	}
	return I;
}

vector<LR1Item> Grammar::go_to(vector<LR1Item> I, const shared_ptr<GrammarSymbol> X) const {
	vector<LR1Item> goto_I_X;
	for (const auto& existingItem : I) {
		const vector<shared_ptr<GrammarSymbol>>& expectedSymbols = existingItem.getExpected();
		if ((!expectedSymbols.empty()) && (expectedSymbols.at(0) == X)) {      // [ A -> a.Xb, c ]
			LR1Item item { existingItem };
			item.advance();

			// XXX: is the if required?
			if (std::find(goto_I_X.begin(), goto_I_X.end(), item) == goto_I_X.end()) {
				goto_I_X.push_back(item);
			}
		}
	}

	return closure(goto_I_X);
}

vector<vector<LR1Item>> Grammar::canonical_collection() const {
	GrammarRuleBuilder ruleBuilder;
	ruleBuilder.setDefiningNonterminal(start_symbol);
	ruleBuilder.addProductionSymbol(this->rules.at(0)->getNonterminal());
	LR1Item initialItem { ruleBuilder.build(), end_symbol };

	vector<LR1Item> initial_set;
	initial_set.push_back(initialItem);
	initial_set = this->closure(initial_set);
	vector<vector<LR1Item>> canonicalCollection;
	canonicalCollection.push_back(initial_set);

	std::vector<std::shared_ptr<GrammarSymbol>> grammarSymbols;
	grammarSymbols.insert(grammarSymbols.begin(), this->terminals.begin(), this->terminals.end());
	grammarSymbols.insert(grammarSymbols.begin(), this->nonterminals.begin(), this->nonterminals.end());

	for (size_t i = 0; i < canonicalCollection.size(); ++i) { // for each set of items I in C
		for (const auto& X : grammarSymbols) { // and each grammar symbol X
			const auto& goto_I_X = go_to(canonicalCollection.at(i), X);
			if (!goto_I_X.empty()) { // such that goto(I, X) is not empty
				if (std::find(canonicalCollection.begin(), canonicalCollection.end(), goto_I_X) == canonicalCollection.end()) { // and not in C
					canonicalCollection.push_back(goto_I_X);
				}
			}
		}
	}
	return canonicalCollection;
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
