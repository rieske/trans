#ifndef _JMP_STMT_NODE_H_
#define _JMP_STMT_NODE_H_

#include <string>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Expression;

class JumpStatement: public parser::NonterminalNode {
public:
	JumpStatement(std::string jumpKeyword, SymbolTable *st, unsigned ln);
	JumpStatement(Expression* expression, SymbolTable *st, unsigned ln);

	string getAttr() const;

	static const std::string ID;

private:
	string attr;
};

}

#endif // _JMP_STMT_NODE_H_