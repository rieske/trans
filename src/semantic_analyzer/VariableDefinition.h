#ifndef VARIABLEDEFINITION_H_
#define VARIABLEDEFINITION_H_

#include <memory>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class Expression;
class VariableDeclaration;

class VariableDefinition: public NonterminalNode {
public:
	VariableDefinition(std::unique_ptr<VariableDeclaration> declaration, std::unique_ptr<Expression> initializerExpression, SymbolTable *st,
			unsigned ln);
	virtual ~VariableDefinition();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	const std::unique_ptr<VariableDeclaration> declaration;
	std::unique_ptr<Expression> initializerExpression;
};

} /* namespace semantic_analyzer */

#endif /* VARIABLEDEFINITION_H_ */
