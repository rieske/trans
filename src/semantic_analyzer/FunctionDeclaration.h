#ifndef FUNCTIONDECLARATION_H_
#define FUNCTIONDECLARATION_H_

#include <memory>
#include <vector>

#include "Declaration.h"
#include "ParameterDeclaration.h"

namespace semantic_analyzer {

class ParameterList;

class FunctionDeclaration: public Declaration {
public:
	FunctionDeclaration(std::unique_ptr<Declaration> declaration, SymbolTable *st);
	FunctionDeclaration(std::unique_ptr<Declaration> declaration, std::unique_ptr<ParameterList> parameterList, SymbolTable *st);
	virtual ~FunctionDeclaration();

	const vector<ParameterDeclaration *> getParams() const;

	void accept(AbstractSyntaxTreeVisitor& visitor) const override;

	const std::unique_ptr<ParameterList> parameterList;
private:
	// FIXME:
	vector<ParameterDeclaration *> params;

};

} /* namespace semantic_analyzer */

#endif /* FUNCTIONDECLARATION_H_ */
