#ifndef _VAR_DECL_NODE_H_
#define _VAR_DECL_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "Declaration.h"

namespace semantic_analyzer {

class DeclarationList;
class Expression;

class VariableDeclaration: public parser::NonterminalNode {
public:
	VariableDeclaration(parser::ParseTreeNode* typeSpecifier, DeclarationList* declarationList, SymbolTable *st, unsigned ln);
	VariableDeclaration(parser::ParseTreeNode* typeSpecifier, DeclarationList* declarationList, Expression* assignmentExpression, SymbolTable *st,
			unsigned ln);

	virtual std::ostringstream &asXml(std::ostringstream &oss, unsigned depth) const;

	static const std::string ID;

private:
	string basic_type;
	vector<Declaration *> decls;
	string init_val;
};

}

#endif // _VAR_DECL_NODE_H_
