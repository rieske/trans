#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include <iostream>
#include <string>

#include "NonterminalNode.h"

class SymbolEntry;
class SymbolTable;

namespace semantic_analyzer {

class Declaration;
class TerminalSymbol;

class ParameterDeclaration: public NonterminalNode {
public:
	ParameterDeclaration(TerminalSymbol typeSpecifier, Declaration* declaration, SymbolTable *st, unsigned ln);

	string getBasicType() const;
	string getExtendedType() const;

	SymbolEntry *getPlace() const;

	static const std::string ID;

private:
	string name;
	string basic_type;
	string extended_type;

	SymbolEntry *place;
};

}

#endif // _PARAM_DECL_NODE_H_
