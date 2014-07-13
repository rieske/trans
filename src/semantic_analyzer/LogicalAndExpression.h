#ifndef _LOG_AND_EXPR_NODE_H_
#define _LOG_AND_EXPR_NODE_H_

#include <string>

#include "LogicalOrExpression.h"

namespace semantic_analyzer {

class LogicalAndExpression: public LogicalOrExpression {
public:
	LogicalAndExpression(LogicalAndExpression* logicalAndExpression, ParseTreeNode* andOperator, Expression* orExpression, SymbolTable *st,
			unsigned ln);
	LogicalAndExpression(Expression* orExpression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _LOG_AND_EXPR_NODE_H_
