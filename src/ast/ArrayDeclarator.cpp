#include "ArrayDeclarator.h"

#include <algorithm>
#include <string>

#include "AbstractSyntaxTreeVisitor.h"

#include "Expression.h"

namespace ast {

ArrayDeclarator::ArrayDeclarator(std::unique_ptr<DirectDeclarator> declarator, std::unique_ptr<Expression> subscriptExpression) :
		DirectDeclarator(declarator->getName(), declarator->getContext()),
		subscriptExpression { std::move(subscriptExpression) } {
}

void ArrayDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} /* namespace ast */
