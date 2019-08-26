#include "TerminalSymbol.h"

namespace ast {

TerminalSymbol::TerminalSymbol(std::string type, std::string value, const translation_unit::Context& context) :
        type { type }, value { value }, context { context } {
}

TerminalSymbol::TerminalSymbol(const TerminalSymbol& that) :
        type { that.type }, value { that.value }, context { that.context } {
}

} /* namespace ast */
