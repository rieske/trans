#include "GrammarSymbol.h"

using std::string;

GrammarSymbol::GrammarSymbol(const string name) :
		name { name } {
}

GrammarSymbol::~GrammarSymbol() {
}

std::string GrammarSymbol::getName() const {
	return name;
}

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol) {
	return ostream << symbol.name;
}
