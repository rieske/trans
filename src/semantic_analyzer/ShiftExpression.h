#ifndef _S_EXPR_NODE_H_
#define _S_EXPR_NODE_H_

#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class TerminalSymbol;

class ShiftExpression: public Expression {
public:
	ShiftExpression(Expression* shiftExpression, TerminalSymbol shiftOperator, Expression* additionExpression, SymbolTable *st,
			unsigned ln);
	ShiftExpression(Expression* additionExpression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _S_EXPR_NODE_H_
