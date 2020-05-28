#include "ReturnStatement.h"

#include <stddef.h>
#include <algorithm>
#include <vector>

#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace ast {

ReturnStatement::ReturnStatement(std::unique_ptr<Expression> returnExpression) :
		returnExpression { std::move(returnExpression) } {
}

void ReturnStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} /* namespace ast */
