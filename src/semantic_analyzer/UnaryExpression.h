#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class TerminalSymbol;

class UnaryExpression: public Expression {
public:
	UnaryExpression(TerminalSymbol unaryOperator, std::unique_ptr<Expression> castExpression, SymbolTable *st);

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

	TerminalSymbol unaryOperator;
	const std::unique_ptr<Expression> castExpression;
};

}

#endif // _U_EXPR_NODE_H_
