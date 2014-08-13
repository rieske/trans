#include "ReturnStatement.h"

#include <stddef.h>
#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace semantic_analyzer {

ReturnStatement::ReturnStatement(std::unique_ptr<Expression> returnExpression) :
		returnExpression { std::move(returnExpression) } {
}

ReturnStatement::~ReturnStatement() {
}

void ReturnStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
