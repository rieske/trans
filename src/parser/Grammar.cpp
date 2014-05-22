#include "Grammar.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>

#include "FirstTable.h"

using std::shared_ptr;
using std::unique_ptr;
using std::vector;

Grammar::Grammar(const vector<shared_ptr<GrammarSymbol>> terminals, const vector<shared_ptr<GrammarSymbol>> nonterminals) :
		terminals { terminals },
		nonterminals { nonterminals },
		start_symbol { std::make_shared<GrammarSymbol>("<__start__>", 0) },
		end_symbol { std::make_shared<GrammarSymbol>("'$end$'", 0) } {
	start_symbol->addProduction( { nonterminals.at(0) });

	firstTable = unique_ptr<FirstTable> { new FirstTable { nonterminals } };
}

Grammar::~Grammar() {
}

LR1Item Grammar::getReductionById(size_t nonterminalId, size_t productionId) const {
	const auto& nonterminalIterator = std::find_if(nonterminals.begin(), nonterminals.end(),
			[&nonterminalId] (shared_ptr<GrammarSymbol> nonterminal) {return nonterminal->getId() == nonterminalId;});
	if (nonterminalIterator == nonterminals.end()) {
		throw std::invalid_argument("Nonterminal not found by id " + nonterminalId);
	}
	return {*nonterminalIterator, productionId, {}};
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

void Grammar::closure(vector<LR1Item>& I) const {
	bool more = true;
	while (more) {
		more = false;
		for (size_t i = 0; i < I.size(); ++i) {
			const LR1Item& item = I.at(i);
			const vector<shared_ptr<GrammarSymbol>>& expectedSymbols = item.getExpected();
			if (!expectedSymbols.empty() && expectedSymbols.at(0)->isNonterminal()) { // [ A -> u.Bv, a ] (expected[0] == B)
				const auto& nextExpectedNonterminal = expectedSymbols.at(0);
				vector<shared_ptr<GrammarSymbol>> firstForNextSymbol {
						(expectedSymbols.size() > 1) ? firstTable->firstSet(expectedSymbols.at(1)) : item.getLookaheads() };
				for (size_t productionId = 0; productionId < nextExpectedNonterminal->getProductions().size(); ++productionId) {
					LR1Item newItem { nextExpectedNonterminal, productionId, firstForNextSymbol };
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

vector<LR1Item> Grammar::goTo(const vector<LR1Item>& I, const shared_ptr<GrammarSymbol> X) const {
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
	closure(goto_I_X);
	return goto_I_X;
}

vector<vector<LR1Item>> Grammar::canonicalCollection() const {
	LR1Item initialItem { start_symbol, 0, { end_symbol } };

	vector<LR1Item> initialSet;
	initialSet.push_back(initialItem);
	closure(initialSet);
	vector<vector<LR1Item>> canonicalCollection;
	canonicalCollection.push_back(initialSet);

	std::vector<std::shared_ptr<GrammarSymbol>> grammarSymbols;
	grammarSymbols.insert(grammarSymbols.begin(), this->terminals.begin(), this->terminals.end());
	grammarSymbols.insert(grammarSymbols.begin(), this->nonterminals.begin(), this->nonterminals.end());

	for (size_t i = 0; i < canonicalCollection.size(); ++i) { // for each set of items I in C
		for (const auto& X : grammarSymbols) { // and each grammar symbol X
			const auto& goto_I_X = goTo(canonicalCollection.at(i), X);
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
