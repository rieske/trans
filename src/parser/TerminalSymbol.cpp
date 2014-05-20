#include "TerminalSymbol.h"

using std::string;

TerminalSymbol::TerminalSymbol(const string value) :
		GrammarSymbol { value } {
}

TerminalSymbol::~TerminalSymbol() {
}
