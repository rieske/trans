#ifndef _OR_EXPR_NODE_H_
#define _OR_EXPR_NODE_H_

#include "Expression.h"

namespace semantic_analyzer {

class BitwiseOrExpression: public Expression {
public:
	BitwiseOrExpression(Expression* bitwiseOrExpression, Expression* xorExpression, SymbolTable *st,
			unsigned ln);
	BitwiseOrExpression(Expression* xorExpression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _OR_EXPR_NODE_H_
