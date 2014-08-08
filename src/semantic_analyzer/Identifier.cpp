#include "Identifier.h"

#include <string>

#include "AbstractSyntaxTreeVisitor.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

Identifier::Identifier(TerminalSymbol identifier) :
        Declaration(identifier.value, "", identifier.line, nullptr) {
}

Identifier::~Identifier() {
}

void Identifier::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
