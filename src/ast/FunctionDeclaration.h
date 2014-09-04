#ifndef FUNCTIONDECLARATION_H_
#define FUNCTIONDECLARATION_H_

#include <memory>
#include <vector>

#include "Declaration.h"
#include "ParameterDeclaration.h"

namespace ast {

class ParameterList;

class FunctionDeclaration: public Declaration {
public:
	FunctionDeclaration(std::unique_ptr<Declaration> declaration);
	FunctionDeclaration(std::unique_ptr<Declaration> declaration, std::unique_ptr<ParameterList> parameterList);
	virtual ~FunctionDeclaration();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	const std::unique_ptr<ParameterList> parameterList;
};

} /* namespace ast */

#endif /* FUNCTIONDECLARATION_H_ */
