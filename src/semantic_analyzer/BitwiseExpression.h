#ifndef _BITWISE_EXPRESSION_NODE_H_
#define _BITWISE_EXPRESSION_NODE_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class BitwiseExpression: public Expression {
public:
	BitwiseExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol bitwiseOperator, std::unique_ptr<Expression> rightHandSide,
			SymbolTable *st);

	void accept(AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string AND;
	static const std::string OR;
	static const std::string XOR;

	std::unique_ptr<Expression> leftHandSide;
	TerminalSymbol bitwiseOperator;
	std::unique_ptr<Expression> rightHandSide;
};

}

#endif // _BITWISE_EXPRESSION_NODE_H_
