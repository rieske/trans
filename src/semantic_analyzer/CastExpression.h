#ifndef _CAST_EXPR_NODE_H_
#define _CAST_EXPR_NODE_H_

#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class TerminalSymbol;
class Pointer;

class CastExpression: public Expression {
public:
	CastExpression(TerminalSymbol typeSpecifier, Expression* castExpression, SymbolTable *st, unsigned ln);
	CastExpression(TerminalSymbol typeSpecifier, Pointer* pointer, Expression* castExpression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _CAST_EXPR_NODE_H_
