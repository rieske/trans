#include "Identifier.h"

#include <string>

#include "AbstractSyntaxTreeVisitor.h"
#include "TerminalSymbol.h"

namespace ast {

Identifier::Identifier(TerminalSymbol identifier) :
        DirectDeclaration(identifier.value, identifier.context) {
}

Identifier::~Identifier() {
}

void Identifier::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
