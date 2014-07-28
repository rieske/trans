#ifndef ARRAYDECLARATION_H_
#define ARRAYDECLARATION_H_

#include <memory>

#include "Declaration.h"

namespace semantic_analyzer {

class Expression;

class ArrayDeclaration: public Declaration {
public:
	ArrayDeclaration(std::unique_ptr<Declaration> declaration, std::unique_ptr<Expression> subscriptExpression, SymbolTable *st,
			unsigned ln);
	virtual ~ArrayDeclaration();

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	const std::unique_ptr<Declaration> declaration;
	const std::unique_ptr<Expression> subscriptExpression;
};

} /* namespace semantic_analyzer */

#endif /* ARRAYDECLARATION_H_ */
