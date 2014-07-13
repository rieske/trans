#include "GrammarSymbol.h"

using std::string;
using std::vector;

namespace parser {

GrammarSymbol::GrammarSymbol(const string symbol) :
		symbol { symbol } {
}

GrammarSymbol::~GrammarSymbol() {
}

void GrammarSymbol::addProduction(Production production) {
	productions.push_back(production);
}

string GrammarSymbol::getSymbol() const {
	return symbol;
}

vector<Production> GrammarSymbol::getProductions() const {
	return productions;
}

bool GrammarSymbol::isTerminal() const {
	return productions.empty();
}

bool GrammarSymbol::isNonterminal() const {
	return !isTerminal();
}

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol) {
	return ostream << symbol.getSymbol();
}

}
