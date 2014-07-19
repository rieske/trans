#ifndef _LOG_EXPR_NODE_H_
#define _LOG_EXPR_NODE_H_

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "Expression.h"

namespace semantic_analyzer {

class LogicalAndExpression;

class LogicalOrExpression: public Expression {
public:
	LogicalOrExpression(LogicalOrExpression* logicalOrExpression, LogicalAndExpression* logicalAndExpression, SymbolTable *st, unsigned ln);
	LogicalOrExpression(LogicalAndExpression* logicalAndExpression, SymbolTable *st, unsigned ln);

	vector<Quadruple *> getBPList() const;

	static const std::string ID;

protected:
	LogicalOrExpression(std::string label, vector<AbstractSyntaxTreeNode *> children, SymbolTable *st, unsigned ln);

	vector<Quadruple *> backpatchList;
};

}

#endif // _LOG_EXPR_NODE_H_
