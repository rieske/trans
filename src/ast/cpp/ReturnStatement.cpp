#include "ast/ReturnStatement.h"

#include <stddef.h>
#include <algorithm>
#include <vector>

#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/Expression.h"

namespace ast {

ReturnStatement::ReturnStatement(std::unique_ptr<Expression> returnExpression) :
		returnExpression { std::move(returnExpression) } {
}

ReturnStatement::~ReturnStatement() {
}

void ReturnStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} /* namespace ast */
