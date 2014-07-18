#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <iostream>
#include <string>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Declaration;
class TerminalSymbol;

class FunctionDeclaration: public parser::NonterminalNode {
public:
	FunctionDeclaration(TerminalSymbol typeSpecifier, Declaration* declaration, parser::ParseTreeNode* block, SymbolTable *s_t, unsigned ln);

	virtual std::ostringstream &asXml(std::ostringstream &oss, unsigned depth) const;

	static const std::string ID;

private:
	string name;
	string basic_type;
	string extended_type;
};

}

#endif // _FUNC_DECL_NODE_H_
