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

const vector<GrammarRule>& GrammarSymbol::getProductionRules() {
	return productionRules;
}

bool GrammarSymbol::isTerminal() {
	return productionRules.empty();
}

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol) {
	return ostream << symbol.getName();
}
