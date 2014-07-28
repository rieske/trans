#include "Identifier.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

Identifier::Identifier(TerminalSymbol identifier) :
		identifier { identifier.value } {
	name = this->identifier;
}

Identifier::~Identifier() {
}

void Identifier::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
