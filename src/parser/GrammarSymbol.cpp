#include "GrammarSymbol.h"

using std::string;
using std::vector;

namespace parser {

GrammarSymbol::GrammarSymbol(const string definition) :
		definition { definition } {
}

GrammarSymbol::~GrammarSymbol() {
}

void GrammarSymbol::addProduction(Production production) {
	productions.push_back(production);
}

string GrammarSymbol::getDefinition() const {
	return definition;
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
	return ostream << symbol.getDefinition();
}

}
