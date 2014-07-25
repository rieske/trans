#ifndef _LOG_EXPR_NODE_H_
#define _LOG_EXPR_NODE_H_

#include <memory>
#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class LogicalOrExpression: public Expression {
public:
	LogicalOrExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide, SymbolTable *st, unsigned ln);

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ID;

	std::unique_ptr<Expression> leftHandSide;
	std::unique_ptr<Expression> rightHandSide;
};

}

#endif // _LOG_EXPR_NODE_H_
