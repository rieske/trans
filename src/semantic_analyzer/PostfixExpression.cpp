#include "PostfixExpression.h"

#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

const std::string PostfixExpression::ID { "<postfix_expr>" };

PostfixExpression::PostfixExpression(std::unique_ptr<Expression> postfixExpression, TerminalSymbol postfixOperator, SymbolTable *st) :
		Expression(ID, { }, st, postfixOperator.line),
		postfixExpression { std::move(postfixExpression) },
		postfixOperator { postfixOperator } {
	saveExpressionAttributes(*this->postfixExpression);
	if (place == NULL || value == "rval") {   // dirbama su konstanta
		semanticError("lvalue required as increment operand\n");
	} else {   // dirbama su kinmamuoju simbolių lentelėje
		if (postfixOperator.value == "++") {
			code.push_back(new Quadruple(INC, place, NULL, place));
		} else if (postfixOperator.value == "--") {
			code.push_back(new Quadruple(DEC, place, NULL, place));
		}
		value = "rval";
	}
}

void PostfixExpression::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
