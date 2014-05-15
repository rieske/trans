#include "FirstTable.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "GrammarRule.h"
#include "GrammarSymbol.h"

using std::vector;
using std::shared_ptr;

FirstTable::FirstTable(const vector<shared_ptr<GrammarRule>>& grammarRules) {
	initializeTable(grammarRules);

	bool moreToAdd = true;
	while (moreToAdd) {
		moreToAdd = false;
		for (const auto& rule : grammarRules) {
			vector<shared_ptr<GrammarSymbol>> production = rule->getProduction();
			shared_ptr<GrammarSymbol> firstProductionSymbol = production.at(0);
			for (const auto& firstSymbol : firstTable.at(firstProductionSymbol)) {
				moreToAdd |= addFirstSymbol(rule->getNonterminal(), firstSymbol);
			}
		}
	}
}

FirstTable::~FirstTable() {
}

bool FirstTable::addFirstSymbol(const shared_ptr<GrammarSymbol>& firstFor, const shared_ptr<GrammarSymbol>& firstSymbol) {
	auto& firstSetForSymbol = firstTable.at(firstFor);
	if (std::find(firstSetForSymbol.begin(), firstSetForSymbol.end(), firstSymbol) == firstSetForSymbol.end()) {
		firstSetForSymbol.push_back(firstSymbol);
		return true;
	}
	return false;
}

void FirstTable::initializeTable(const vector<shared_ptr<GrammarRule>>& grammarRules) {
	for (const auto& rule : grammarRules) {
		if (firstTable.find(rule->getNonterminal()) == firstTable.end()) {
			firstTable[rule->getNonterminal()] = vector<shared_ptr<GrammarSymbol>> { };
		}
		for (const auto& productionSymbol : rule->getProduction()) {
			if (firstTable.find(productionSymbol) == firstTable.end()) {
				firstTable[productionSymbol] = vector<shared_ptr<GrammarSymbol>> { };
			}
			if (productionSymbol->isTerminal()) {
				addFirstSymbol(productionSymbol, productionSymbol);
			}
		}
	}
}

const vector<shared_ptr<GrammarSymbol>> FirstTable::firstSet(const shared_ptr<GrammarSymbol> symbol) {
	return firstTable.at(symbol);
}

std::ostream& operator<<(std::ostream& ostream, const FirstTable& firstTable) {
	for (const auto& symbolFirstSet : firstTable.firstTable) {
		ostream << "FIRST(" << *symbolFirstSet.first << "):\t";
		for (const auto& firstSymbol : symbolFirstSet.second) {
			ostream << *firstSymbol << " ";
		}
		ostream << "\n";
	}
	return ostream;
}
