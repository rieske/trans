#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include <iostream>
#include <string>

#include "../parser/NonterminalNode.h"

class SymbolEntry;
class SymbolTable;

namespace semantic_analyzer {

class Declaration;

class ParameterDeclaration: public NonterminalNode {
public:
	ParameterDeclaration(ParseTreeNode* typeSpecifier, Declaration* declaration, SymbolTable *st, unsigned ln);

	string getBasicType() const;
	string getExtendedType() const;

	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

	void output_attr(ostringstream &oss, unsigned nr) const;

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
