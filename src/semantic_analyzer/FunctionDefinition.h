#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <iostream>
#include <string>

#include "NonterminalNode.h"

class SymbolTable;

namespace semantic_analyzer {

class Declaration;
class TerminalSymbol;

class FunctionDefinition: public NonterminalNode {
public:
	FunctionDefinition(TerminalSymbol typeSpecifier, Declaration* declaration, AbstractSyntaxTreeNode* block, SymbolTable *s_t, unsigned ln);

	static const std::string ID;

private:
	string name;
	string basic_type;
	string extended_type;
};

}

#endif // _FUNC_DECL_NODE_H_
