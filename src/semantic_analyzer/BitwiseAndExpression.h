#ifndef _AND_EXPR_NODE_H_
#define _AND_EXPR_NODE_H_

#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class BitwiseAndExpression: public Expression {
public:
	BitwiseAndExpression(Expression* andExpression, Expression* equalityExpression, SymbolTable *st, unsigned ln);
	BitwiseAndExpression(Expression* equalityExpression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _AND_EXPR_NODE_H_
