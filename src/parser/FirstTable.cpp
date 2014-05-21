#include "FirstTable.h"

#include <algorithm>
#include <iterator>
#include <utility>

//#include "GrammarRule.h"
#include "GrammarSymbol.h"

using std::vector;
using std::shared_ptr;

FirstTable::FirstTable(const vector<shared_ptr<GrammarSymbol>>& nonterminals) {
	initializeTable(nonterminals);

	bool moreToAdd = true;
	while (moreToAdd) {
		moreToAdd = false;
		for (const auto& nonterminal : nonterminals) {
			for (const auto& productionRule : nonterminal->getProductionRules()) {
				shared_ptr<GrammarSymbol> firstProductionSymbol = productionRule.getProduction().at(0);
				for (const auto& firstSymbol : firstTable.at(firstProductionSymbol)) {
					moreToAdd |= addFirstSymbol(nonterminal, firstSymbol);
				}
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

void FirstTable::initializeTable(const vector<shared_ptr<GrammarSymbol>>& symbols) {
	for (const auto& symbol : symbols) {
		if (firstTable.find(symbol) == firstTable.end()) {
			firstTable[symbol] = vector<shared_ptr<GrammarSymbol>> { };
		}
		for (const auto& productionRule : symbol->getProductionRules()) {
			for (const auto& productionSymbol : productionRule.getProduction()) {
				if (firstTable.find(productionSymbol) == firstTable.end()) {
					firstTable[productionSymbol] = vector<shared_ptr<GrammarSymbol>> { };
				}
				if (productionSymbol->isTerminal()) {
					addFirstSymbol(productionSymbol, productionSymbol);
				}
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
