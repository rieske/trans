#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class TerminalSymbol;
class CastExpression;

class UnaryExpression: public Expression {
public:
	UnaryExpression(TerminalSymbol incrementOperator, UnaryExpression* unaryExpression, SymbolTable *st, unsigned ln);
	UnaryExpression(TerminalSymbol unaryOperator, CastExpression* castExpression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _U_EXPR_NODE_H_
