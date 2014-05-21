#include "TerminalSymbol.h"

using std::string;

TerminalSymbol::TerminalSymbol(const string value, const size_t id) :
		GrammarSymbol { value, id } {
}

TerminalSymbol::~TerminalSymbol() {
}
