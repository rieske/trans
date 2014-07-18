#ifndef _ADD_EXPR_NODE_H_
#define _ADD_EXPR_NODE_H_

#include "Expression.h"

namespace semantic_analyzer {

class TerminalSymbol;

class AdditionExpression: public Expression {
public:
	AdditionExpression(Expression* addExpression, TerminalSymbol additionOperator, Expression* factor, SymbolTable *st, unsigned ln);
	AdditionExpression(Expression* factor, SymbolTable *st, unsigned ln);

	static const std::string ID;
private:
};

}

#endif // _ADD_EXPR_NODE_H_
