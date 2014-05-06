#include "NonterminalSymbol.h"

using std::string;

NonterminamSymbol::NonterminamSymbol(const string value) :
		GrammarSymbol { value } {
}

NonterminamSymbol::~NonterminamSymbol() {
}

bool NonterminamSymbol::isTerminal() const {
	return false;
}
