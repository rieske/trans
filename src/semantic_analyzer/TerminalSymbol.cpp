#include "TerminalSymbol.h"

namespace semantic_analyzer {

TerminalSymbol::TerminalSymbol(std::string type, std::string value, size_t line) :
		type { type },
		value { value },
		line { line } {
}

TerminalSymbol::TerminalSymbol(const TerminalSymbol& that) :
		type { that.type },
		value { that.value },
		line { that.line } {
}

TerminalSymbol::~TerminalSymbol() {
}

} /* namespace semantic_analyzer */
