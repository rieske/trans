#ifndef _EQ_EXPR_NODE_H_
#define _EQ_EXPR_NODE_H_

#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class TerminalSymbol;
class ComparisonExpression;

class EqualityExpression: public Expression {
public:
	EqualityExpression(EqualityExpression* equalityExpression, TerminalSymbol equalityOperator, ComparisonExpression* comparisonExpression,
			SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _EQ_EXPR_NODE_H_
