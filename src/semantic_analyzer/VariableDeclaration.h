#ifndef _VAR_DECL_NODE_H_
#define _VAR_DECL_NODE_H_

#include <memory>
#include <string>

#include "NonterminalNode.h"
#include "TypeSpecifier.h"

namespace semantic_analyzer {

class DeclarationList;
class Expression;
class TerminalSymbol;

class VariableDeclaration: public NonterminalNode {
public:
	VariableDeclaration(TypeSpecifier typeSpecifier, std::unique_ptr<DeclarationList> declarationList, SymbolTable *st);

	static const std::string ID;

	void accept(AbstractSyntaxTreeVisitor& visitor) const override;

	const TypeSpecifier declaredType;
	const std::unique_ptr<DeclarationList> declaredVariables;
private:

};

}

#endif // _VAR_DECL_NODE_H_
