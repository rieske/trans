#include "FirstTable.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "GrammarSymbol.h"

using std::vector;
using std::unique_ptr;

FirstTable::FirstTable(const vector<const GrammarSymbol*>& symbols) {
	initializeTable(symbols);

	bool moreToAdd = true;
	while (moreToAdd) {
		moreToAdd = false;
		for (const auto& symbol : symbols) {
			for (const auto& production : symbol->getProductions()) {
				const GrammarSymbol* firstProductionSymbol = production.at(0);
				for (const auto& firstSymbol : firstTable.at(firstProductionSymbol)) {
					moreToAdd |= addFirstSymbol(symbol, firstSymbol);
				}
			}
		}
	}
}

FirstTable::~FirstTable() {
}

const vector<const GrammarSymbol*> FirstTable::operator()(const GrammarSymbol* symbol) const {
	return firstTable.at(symbol);
}

bool FirstTable::addFirstSymbol(const GrammarSymbol* firstFor, const GrammarSymbol* firstSymbol) {
	auto& firstSetForSymbol = firstTable.at(firstFor);
	if (std::find(firstSetForSymbol.begin(), firstSetForSymbol.end(), firstSymbol) == firstSetForSymbol.end()) {
		firstSetForSymbol.push_back(firstSymbol);
		return true;
	}
	return false;
}

void FirstTable::initializeTable(const vector<const GrammarSymbol*>& symbols) {
	for (const auto& symbol : symbols) {
		if (firstTable.find(symbol) == firstTable.end()) {
			firstTable[symbol] = vector<const GrammarSymbol*> { };
		}
		for (const auto& production : symbol->getProductions()) {
			for (const auto& productionSymbol : production) {
				if (firstTable.find(productionSymbol) == firstTable.end()) {
					firstTable[productionSymbol] = vector<const GrammarSymbol*> { };
				}
				if (productionSymbol->isTerminal()) {
					addFirstSymbol(productionSymbol, productionSymbol);
				}
			}
		}
	}
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
