#ifndef _ML_EXPR_NODE_H_
#define _ML_EXPR_NODE_H_

#include "Expression.h"

namespace semantic_analyzer {

class TerminalSymbol;

class ComparisonExpression: public Expression {
public:
	ComparisonExpression(Expression* comparisonExpression, TerminalSymbol comparisonOperator, Expression* shiftExpression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _ML_EXPR_NODE_H_
