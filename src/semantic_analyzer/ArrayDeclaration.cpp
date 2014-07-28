#include "ArrayDeclaration.h"

#include <algorithm>
#include <string>

#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace semantic_analyzer {

ArrayDeclaration::ArrayDeclaration(std::unique_ptr<Declaration> declaration, std::unique_ptr<Expression> subscriptExpression,
		SymbolTable *st, unsigned ln) :
		Declaration(st, ln),
		declaration { std::move(declaration) },
		subscriptExpression { std::move(subscriptExpression) } {
	// FIXME: not implemented
	name = this->declaration->getName();
	type = "a";
}

ArrayDeclaration::~ArrayDeclaration() {
}

void ArrayDeclaration::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
