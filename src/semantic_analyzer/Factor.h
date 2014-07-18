#ifndef _FACTOR_NODE_H_
#define _FACTOR_NODE_H_

#include "Expression.h"

namespace semantic_analyzer {

class TerminalSymbol;

class Factor: public Expression {
public:
	Factor(Expression* factor, TerminalSymbol multiplicationOperator, Expression* castExpression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _FACTOR_NODE_H_
