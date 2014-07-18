#ifndef _XOR_EXPR_NODE_H_
#define _XOR_EXPR_NODE_H_

#include "Expression.h"

namespace semantic_analyzer {

class BitwiseXorExpression: public Expression {
public:
	BitwiseXorExpression(Expression* bitwiseXorExpression, Expression* bitwiseAndExpression,
			SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _XOR_EXPR_NODE_H_
