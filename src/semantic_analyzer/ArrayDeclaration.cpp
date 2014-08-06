#include "ArrayDeclaration.h"

#include <algorithm>
#include <string>

#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace semantic_analyzer {

ArrayDeclaration::ArrayDeclaration(std::unique_ptr<Declaration> declaration, std::unique_ptr<Expression> subscriptExpression,
		SymbolTable *st) :
		Declaration(declaration->getName(), "a" + declaration->getType(), declaration->getLineNumber(), st),
		subscriptExpression { std::move(subscriptExpression) } {
}

ArrayDeclaration::~ArrayDeclaration() {
}

void ArrayDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
