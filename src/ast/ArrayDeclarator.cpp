#include "ArrayDeclarator.h"

#include <algorithm>
#include <string>

#include "AbstractSyntaxTreeVisitor.h"

#include "Expression.h"

namespace ast {

ArrayDeclarator::ArrayDeclarator(std::unique_ptr<Declarator> declaration, std::unique_ptr<Expression> subscriptExpression) :
		DirectDeclarator(declaration->getName(), declaration->getContext()),
		subscriptExpression { std::move(subscriptExpression) } {
}

ArrayDeclarator::~ArrayDeclarator() {
}

void ArrayDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} /* namespace ast */
