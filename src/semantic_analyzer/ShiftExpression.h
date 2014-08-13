#ifndef _S_EXPR_NODE_H_
#define _S_EXPR_NODE_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class TerminalSymbol;

class ShiftExpression: public Expression {
public:
	ShiftExpression(std::unique_ptr<Expression> shiftExpression, TerminalSymbol shiftOperator,
			std::unique_ptr<Expression> additionExpression);

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

	std::unique_ptr<Expression> shiftExpression;
	TerminalSymbol shiftOperator;
	std::unique_ptr<Expression> additionExpression;
};

}

#endif // _S_EXPR_NODE_H_
