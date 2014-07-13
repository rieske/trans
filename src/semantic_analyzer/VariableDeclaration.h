#ifndef _VAR_DECL_NODE_H_
#define _VAR_DECL_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "Declaration.h"

namespace semantic_analyzer {

class DeclarationList;
class Expression;

class VariableDeclaration: public NonterminalNode {
public:
	VariableDeclaration(ParseTreeNode* typeSpecifier, DeclarationList* declarationList, ParseTreeNode* semicolon, SymbolTable *st,
			unsigned ln);
	VariableDeclaration(ParseTreeNode* typeSpecifier, DeclarationList* declarationList, ParseTreeNode* assignmentOperator,
			Expression* assignmentExpression, ParseTreeNode* semicolon, SymbolTable *st, unsigned ln);

	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

	static const std::string ID;

private:
	string basic_type;
	vector<Declaration *> decls;
	string init_val;
};

}

#endif // _VAR_DECL_NODE_H_
