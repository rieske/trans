#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <memory>
#include <string>

#include "BasicType.h"
#include "TypeSpecifier.h"
#include "NonterminalNode.h"

class SymbolTable;

namespace semantic_analyzer {

class FunctionDeclaration;
class TerminalSymbol;

class FunctionDefinition: public NonterminalNode {
public:
	FunctionDefinition(TypeSpecifier returnType, std::unique_ptr<FunctionDeclaration> declaration,
			std::unique_ptr<AbstractSyntaxTreeNode> body, SymbolTable *s_t);
	virtual ~FunctionDefinition();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

	TypeSpecifier returnType;
	const std::unique_ptr<FunctionDeclaration> declaration;
	const std::unique_ptr<AbstractSyntaxTreeNode> body;

private:
	string name;
	BasicType basicType;
	string extended_type;
};

}

#endif // _FUNC_DECL_NODE_H_
