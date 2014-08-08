#ifndef _A_EXPR_NODE_H_
#define _A_EXPR_NODE_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class TerminalSymbol;
class LogicalOrExpression;

class AssignmentExpression: public Expression {
public:
	AssignmentExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol assignmentOperator,
			std::unique_ptr<Expression> rightHandSide, SymbolTable *st);

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

	const std::unique_ptr<Expression> leftHandSide;
	const TerminalSymbol assignmentOperator;
	const std::unique_ptr<Expression> rightHandSide;
};

}

#endif // _A_EXPR_NODE_H_
