#ifndef _A_EXPR_NODE_H_
#define _A_EXPR_NODE_H_

#include <iostream>
#include <string>

#include "LogicalOrExpression.h"

namespace semantic_analyzer {

class AssignmentExpression: public LogicalOrExpression {
public:
	AssignmentExpression(Expression* unaryExpression, std::string assignmentOperator, Expression* assignmentExpression, SymbolTable *st,
			unsigned ln);
	AssignmentExpression(LogicalOrExpression* logExpr, SymbolTable *st, unsigned ln);

	void output_attr(std::ostringstream &oss, unsigned nr) const;

	static const std::string ID;
private:
};

}

#endif // _A_EXPR_NODE_H_
