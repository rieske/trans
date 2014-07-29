#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <memory>
#include <string>

#include "NonterminalNode.h"
#include "TerminalSymbol.h"

class SymbolTable;

namespace semantic_analyzer {

class FunctionDeclaration;
class TerminalSymbol;

class FunctionDefinition: public NonterminalNode {
public:
	FunctionDefinition(TerminalSymbol typeSpecifier, std::unique_ptr<FunctionDeclaration> declaration,
			std::unique_ptr<AbstractSyntaxTreeNode> body, SymbolTable *s_t);
	virtual ~FunctionDefinition();

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ID;

	const TerminalSymbol typeSpecifier;
	const std::unique_ptr<FunctionDeclaration> declaration;
	const std::unique_ptr<AbstractSyntaxTreeNode> body;

private:
	string name;
	string basic_type;
	string extended_type;
};

}

#endif // _FUNC_DECL_NODE_H_
