#ifndef _LOG_AND_EXPR_NODE_H_
#define _LOG_AND_EXPR_NODE_H_

#include <memory>
#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class LogicalAndExpression: public Expression {
public:
	LogicalAndExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide, SymbolTable *st, unsigned ln);

	void accept(AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ID;

	const std::unique_ptr<Expression> leftHandSide;
	const std::unique_ptr<Expression> rightHandSide;
};

}

#endif // _LOG_AND_EXPR_NODE_H_
