#include "GrammarSymbol.h"

using std::string;
using std::vector;

GrammarSymbol::GrammarSymbol(const string name) :
		name { name } {
}

GrammarSymbol::~GrammarSymbol() {
}

string GrammarSymbol::getName() const {
	return name;
}

const vector<Production>& GrammarSymbol::getProductions() {
	return productions;
}

bool GrammarSymbol::isTerminal() {
	return productions.empty();
}

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol) {
	return ostream << symbol.getName();
}
