#include "NonterminalSymbol.h"

#include <vector>

using std::string;

NonterminalSymbol::NonterminalSymbol(const string value) :
		GrammarSymbol { value } {
}

NonterminalSymbol::~NonterminalSymbol() {
}

void NonterminalSymbol::addProduction(Production production) {
	productions.push_back(production);
}
