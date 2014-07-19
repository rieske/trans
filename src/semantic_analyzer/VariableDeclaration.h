#ifndef _VAR_DECL_NODE_H_
#define _VAR_DECL_NODE_H_

#include <string>
#include <vector>

#include "Declaration.h"

namespace semantic_analyzer {

class DeclarationList;
class Expression;
class TerminalSymbol;

class VariableDeclaration: public NonterminalNode {
public:
	VariableDeclaration(TerminalSymbol typeSpecifier, DeclarationList* declarationList, SymbolTable *st, unsigned ln);
	VariableDeclaration(TerminalSymbol typeSpecifier, DeclarationList* declarationList, Expression* assignmentExpression, SymbolTable *st,
			unsigned ln);

	static const std::string ID;

private:
	string basic_type;
	vector<Declaration *> decls;
	string init_val;
};

}

#endif // _VAR_DECL_NODE_H_
