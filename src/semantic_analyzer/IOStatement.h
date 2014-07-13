#ifndef _IO_STMT_NODE_H_
#define _IO_STMT_NODE_H_

#include <string>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Expression;

class IOStatement: public NonterminalNode {
public:
	IOStatement(ParseTreeNode* ioKeyword, Expression* expression, ParseTreeNode* semicolon, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _IO_STMT_NODE_H_
