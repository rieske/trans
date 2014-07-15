#ifndef _EQ_EXPR_NODE_H_
#define _EQ_EXPR_NODE_H_

#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class ComparisonExpression;

class EqualityExpression: public Expression {
public:
	EqualityExpression(EqualityExpression* equalityExpression, std::string equalityOperator, ComparisonExpression* comparisonExpression, SymbolTable *st,
			unsigned ln);
	EqualityExpression(ComparisonExpression* comparisonExpression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _EQ_EXPR_NODE_H_
