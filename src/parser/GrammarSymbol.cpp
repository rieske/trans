#include "GrammarSymbol.h"

using std::string;
using std::vector;

GrammarSymbol::GrammarSymbol(const string name, const size_t id) :
		id { id },
		name { name } {
}

GrammarSymbol::~GrammarSymbol() {
}

void GrammarSymbol::addProduction(Production production) {
	productions.push_back(production);
}

size_t GrammarSymbol::getId() const {
	return id;
}

string GrammarSymbol::getName() const {
	return name;
}

const vector<Production>& GrammarSymbol::getProductions() const {
	return productions;
}

bool GrammarSymbol::isTerminal() const {
	return productions.empty();
}

bool GrammarSymbol::isNonterminal() const {
	return !isTerminal();
}

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol) {
	return ostream << symbol.getName();
}
