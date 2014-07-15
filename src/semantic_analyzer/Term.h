#ifndef _TERM_NODE_H_
#define _TERM_NODE_H_

#include "Expression.h"

namespace semantic_analyzer {

class Term: public Expression {
public:
	Term(ParseTreeNode* termLiteral, std::string termType, SymbolTable *st, unsigned ln);
	Term(Expression* expression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _TERM_NODE_H_
