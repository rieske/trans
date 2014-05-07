#include "NonterminalSymbol.h"

using std::string;

NonterminalSymbol::NonterminalSymbol(const string value) :
		GrammarSymbol { value } {
}

NonterminalSymbol::~NonterminalSymbol() {
}

bool NonterminalSymbol::isTerminal() const {
	return false;
}
