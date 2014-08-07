#include "PrefixExpression.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

PrefixExpression::PrefixExpression(TerminalSymbol incrementOperator, std::unique_ptr<Expression> unaryExpression, SymbolTable *st) :
		Expression(st, incrementOperator.line),
		unaryExpression { std::move(unaryExpression) },
		incrementOperator { incrementOperator } {
	saveExpressionAttributes(*this->unaryExpression);
	vector<Quadruple *>::iterator it = code.begin();
	if (resultPlace == NULL || value == "rval") {   // dirbama su konstanta
		semanticError("lvalue required as increment operand\n");
	} else {
		if (incrementOperator.value == "++") {
			code.insert(it, new Quadruple(INC, resultPlace, NULL, resultPlace));
		} else if (incrementOperator.value == "--") {
			code.insert(it, new Quadruple(DEC, resultPlace, NULL, resultPlace));
		} else {
			semanticError("Unidentified increment operator: " + incrementOperator.value);
		}
		value = "rval";
	}
}

PrefixExpression::~PrefixExpression() {
}

void PrefixExpression::accept(AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
