#include "NonterminalSymbol.h"

#include <vector>

using std::string;

NonterminalSymbol::NonterminalSymbol(const string value) :
		GrammarSymbol { value } {
}

NonterminalSymbol::~NonterminalSymbol() {
}

void NonterminalSymbol::addProductionRule(GrammarRule productionRule) {
	productionRules.push_back(productionRule);
}
