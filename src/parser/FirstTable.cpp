#include "FirstTable.h"

#include <algorithm>
#include <iterator>

#include "GrammarRule.h"
#include "GrammarSymbol.h"

using std::vector;
using std::shared_ptr;

FirstTable::FirstTable(const vector<shared_ptr<GrammarRule>>& grammarRules) {
	bool more = true;
	while (more) {
		more = false;
		for (unsigned j = 1; j < grammarRules.size(); ++j) {
			auto& rule = grammarRules.at(j);
			vector<shared_ptr<GrammarSymbol>> production = rule->getProduction();
			for (auto& productionSymbol : production) {
				if (productionSymbol->isTerminal()) {
					addFirst(productionSymbol, productionSymbol);
				}
				shared_ptr<GrammarSymbol> firstSymbol = production.at(0);
				if (firstSymbol->isTerminal()) {
					more |= addFirst(rule->getNonterminal(), firstSymbol);    // jei tokio dar nebuvo
				} else {
					more |= addFirstRow(rule->getNonterminal(), firstSymbol);
				}
			}
		}
	}
}

FirstTable::~FirstTable() {
}

bool FirstTable::addFirst(shared_ptr<GrammarSymbol> symbol, shared_ptr<GrammarSymbol> first) {
	if (firstTable.find(symbol) == firstTable.end()) {
		firstTable[symbol] = vector<shared_ptr<GrammarSymbol>> { };
	}
	auto& firstForNonterminal = firstTable.at(symbol);
	if (std::find(firstForNonterminal.begin(), firstForNonterminal.end(), first) == firstForNonterminal.end()) {
		firstForNonterminal.push_back(first);
		return true;
	}
	return false;
}

bool FirstTable::addFirstRow(shared_ptr<GrammarSymbol> dest, shared_ptr<GrammarSymbol> src) {
	bool ret = false;
	if (firstTable.find(src) == firstTable.end()) {
		firstTable[src] = vector<shared_ptr<GrammarSymbol>> { };
	}
	for (auto& firstSymbol : firstTable.at(src)) {
		if (addFirst(dest, firstSymbol))
			ret = true;
	}
	return ret;
}

const vector<std::shared_ptr<GrammarSymbol>> FirstTable::firstSet(const shared_ptr<GrammarSymbol> symbol) {
	return firstTable.at(symbol);
}
