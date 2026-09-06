#include "TerminalSymbol.h"

#include <utility>

namespace ast {

TerminalSymbol::TerminalSymbol(std::string value, const translation_unit::Context& context) :
        value { std::move(value) }, context { context } {
}

} // namespace ast
