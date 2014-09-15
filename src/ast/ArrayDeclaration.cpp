#include "ArrayDeclaration.h"

#include <algorithm>
#include <string>

#include "AbstractSyntaxTreeVisitor.h"

#include "Expression.h"

namespace ast {

ArrayDeclaration::ArrayDeclaration(std::unique_ptr<Declaration> declaration, std::unique_ptr<Expression> subscriptExpression) :
		Declaration(declaration->getName(), declaration->getContext()),
		subscriptExpression { std::move(subscriptExpression) } {
}

ArrayDeclaration::~ArrayDeclaration() {
}

void ArrayDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} /* namespace ast */
