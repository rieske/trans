#ifndef _JMP_STMT_NODE_H_
#define _JMP_STMT_NODE_H_

#include <string>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Expression;

class JumpStatement: public NonterminalNode {
public:
	JumpStatement(ParseTreeNode* jumpKeyword, ParseTreeNode* semicolon, SymbolTable *st, unsigned ln);
	JumpStatement(ParseTreeNode* returnKeyword, Expression* expression, ParseTreeNode* semicolon, SymbolTable *st, unsigned ln);

	string getAttr() const;

	static const std::string ID;

private:
	string attr;
};

}

#endif // _JMP_STMT_NODE_H_
