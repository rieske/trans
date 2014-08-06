#ifndef _POSTFIX_EXPR_NODE_H_
#define _POSTFIX_EXPR_NODE_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class AssignmentExpressionList;
class Term;

class PostfixExpression: public Expression {
public:
	PostfixExpression(std::unique_ptr<Expression> postfixExpression, TerminalSymbol postfixOperator, SymbolTable *st);

	void accept(AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ID;

	const std::unique_ptr<Expression> postfixExpression;
	const TerminalSymbol postfixOperator;
};

}

#endif // _POSTFIX_EXPR_NODE_H_
