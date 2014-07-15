#ifndef _POSTFIX_EXPR_NODE_H_
#define _POSTFIX_EXPR_NODE_H_

#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class AssignmentExpressionList;
class Term;

class PostfixExpression: public Expression {
public:
	PostfixExpression(Expression* postfixExpression, Expression* expression, SymbolTable *st, unsigned ln);
	PostfixExpression(Expression* postfixExpression, AssignmentExpressionList* assignmentExpressionList, SymbolTable *st, unsigned ln);
	PostfixExpression(Expression* postfixExpression, SymbolTable *st, unsigned ln);
	PostfixExpression(Expression* postfixExpression, std::string postfixOperator, SymbolTable *st, unsigned ln);
	PostfixExpression(Term* term, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _POSTFIX_EXPR_NODE_H_
