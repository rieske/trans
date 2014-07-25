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
	code = this->returnExpression->getCode();
	SymbolEntry *expr_val = this->returnExpression->getPlace();
	code.push_back(new Quadruple(RETURN, expr_val, NULL, NULL));
	// XXX: ??? attr = expr_val->getBasicType() + expr_val->getExtendedType();
}

ReturnStatement::~ReturnStatement() {
}

void ReturnStatement::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
