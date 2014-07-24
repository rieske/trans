#ifndef _ADD_EXPR_NODE_H_
#define _ADD_EXPR_NODE_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class ArithmeticExpression: public Expression {
public:
	ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol arithmeticOperator,
			std::unique_ptr<Expression> rightHandSide, SymbolTable *st);

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ADDITION;
	static const std::string MULTIPLICATION;

	std::unique_ptr<Expression> leftHandSide;
	TerminalSymbol arithmeticOperator;
	std::unique_ptr<Expression> rightHandSide;
};

}

#endif // _ADD_EXPR_NODE_H_
