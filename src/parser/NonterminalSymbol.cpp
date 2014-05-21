#include "NonterminalSymbol.h"

#include <vector>

using std::string;

NonterminalSymbol::NonterminalSymbol(const string value, const size_t id) :
		GrammarSymbol { value, id } {
}

NonterminalSymbol::~NonterminalSymbol() {
}

void NonterminalSymbol::addProduction(Production production) {
	productions.push_back(production);
}
