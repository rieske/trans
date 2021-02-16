#include "ReturnStatement.h"

#include <vector>

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ReturnStatement::ReturnStatement(std::unique_ptr<Expression> returnExpression) :
		returnExpression { std::move(returnExpression) } {
}

void ReturnStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} // namespace ast

