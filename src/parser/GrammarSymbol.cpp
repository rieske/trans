#include "GrammarSymbol.h"

using std::string;

GrammarSymbol::GrammarSymbol(const string value) :
		value { value } {
}

bool GrammarSymbol::operator<(const GrammarSymbol& rhs) const {
	return value < rhs.value;
}
