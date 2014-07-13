#ifndef _FACTOR_NODE_H_
#define _FACTOR_NODE_H_

#include "Expression.h"

namespace semantic_analyzer {

class Factor: public Expression {
public:
	Factor(Expression* factor, ParseTreeNode* multiplicationOperator, Expression* castExpression, SymbolTable *st, unsigned ln);
	Factor(Expression* castExpression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _FACTOR_NODE_H_
