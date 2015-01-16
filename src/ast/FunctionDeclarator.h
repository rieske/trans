#ifndef FUNCTIONDECLARATION_H_
#define FUNCTIONDECLARATION_H_

#include <memory>
#include <vector>

#include "DirectDeclarator.h"
#include "FormalArgument.h"

namespace ast {

class ParameterList;

class FunctionDeclarator: public DirectDeclarator {
public:
	FunctionDeclarator(std::unique_ptr<Declarator> declaration);
	FunctionDeclarator(std::unique_ptr<Declarator> declaration, std::unique_ptr<ParameterList> parameterList);
	virtual ~FunctionDeclarator();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	const std::unique_ptr<ParameterList> formalArguments;
};

} /* namespace ast */

#endif /* FUNCTIONDECLARATION_H_ */
