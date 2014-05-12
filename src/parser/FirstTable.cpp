#include "FirstTable.h"

#include <algorithm>
#include <iterator>
#include <cassert>

#include "GrammarRule.h"
#include "GrammarSymbol.h"

using std::vector;
using std::shared_ptr;

FirstTable::FirstTable(const vector<shared_ptr<GrammarRule>>& grammarRules) {
	bool more = false;

	do {
		more = false;
		for (unsigned j = 1; j < grammarRules.size(); ++j) {
			auto& rule = grammarRules.at(j);
			vector<shared_ptr<GrammarSymbol>> production = rule->getProduction();
			for (unsigned i = 0; i < production.size(); ++i) {
				shared_ptr<GrammarSymbol> firstSymbol = production.at(0);
				if (firstSymbol->isTerminal()) {
					if (addFirst(rule->getNonterminal(), firstSymbol))     // jei tokio dar nebuvo
						more = true;
					break;
				} else {
					if (addFirstRow(rule->getNonterminal(), firstSymbol))
						more = true;
					break;
				}
			}
		}
	} while (more);
}

FirstTable::~FirstTable() {
}

bool FirstTable::addFirst(shared_ptr<GrammarSymbol> nonterminal, shared_ptr<GrammarSymbol> first) {
	if (firstTable.find(nonterminal) == firstTable.end()) {
		firstTable[nonterminal] = vector<shared_ptr<GrammarSymbol>> { };
	}
	auto& firstForNonterminal = firstTable.at(nonterminal);
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

const vector<std::shared_ptr<GrammarSymbol>> FirstTable::firstSetForNonterminal(const shared_ptr<GrammarSymbol> nonterminal) {
	assert(!nonterminal->isTerminal());
	return firstTable.at(nonterminal);
}
