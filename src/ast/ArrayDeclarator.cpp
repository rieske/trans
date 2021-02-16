#include "ArrayDeclarator.h"

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

std::unique_ptr<FundamentalType> ArrayDeclarator::getFundamentalType(std::vector<Pointer> indirection, const FundamentalType& baseType){
    throw std::runtime_error {"Arrays not implemented yet"};
}

} // namespace ast
