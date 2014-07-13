#ifndef _POSTFIX_EXPR_NODE_H_
#define _POSTFIX_EXPR_NODE_H_

#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class AssignmentExpressionList;

class PostfixExpression: public Expression {
public:
	PostfixExpression(Expression* postfixExpression, ParseTreeNode* openBrace, Expression* expression, ParseTreeNode* closeBrace,
			SymbolTable *st, unsigned ln);
	PostfixExpression(Expression* postfixExpression, ParseTreeNode* openParenthesis, AssignmentExpressionList* assignmentExpressionList,
			ParseTreeNode* closeParenthesis, SymbolTable *st, unsigned ln);
	PostfixExpression(Expression* postfixExpression, ParseTreeNode* openParenthesis, ParseTreeNode* closeParenthesis, SymbolTable *st,
			unsigned ln);
	PostfixExpression(Expression* postfixExpression, ParseTreeNode* postfixOperator, SymbolTable *st, unsigned ln);
	PostfixExpression(Expression* term, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _POSTFIX_EXPR_NODE_H_
