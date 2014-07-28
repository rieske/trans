#ifndef NOARGFUNCTIONDECLARATION_H_
#define NOARGFUNCTIONDECLARATION_H_

#include <memory>

#include "Declaration.h"

namespace semantic_analyzer {

class NoArgFunctionDeclaration: public Declaration {
public:
	NoArgFunctionDeclaration(std::unique_ptr<Declaration> declaration, SymbolTable *st, unsigned ln);
	virtual ~NoArgFunctionDeclaration();

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	const std::unique_ptr<Declaration> declaration;
};

} /* namespace semantic_analyzer */

#endif /* NOARGFUNCTIONDECLARATION_H_ */
