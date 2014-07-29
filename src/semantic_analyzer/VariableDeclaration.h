#ifndef _VAR_DECL_NODE_H_
#define _VAR_DECL_NODE_H_

#include <memory>
#include <string>

#include "NonterminalNode.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class DeclarationList;
class Expression;
class TerminalSymbol;

class VariableDeclaration: public NonterminalNode {
public:
	VariableDeclaration(TerminalSymbol typeSpecifier, std::unique_ptr<DeclarationList> declarationList, SymbolTable *st);

	static const std::string ID;

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	const TerminalSymbol typeSpecifier;
	const std::unique_ptr<DeclarationList> declarationList;
private:
	string basic_type;
};

}

#endif // _VAR_DECL_NODE_H_
