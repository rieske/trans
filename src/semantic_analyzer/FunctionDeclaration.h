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
	FunctionDeclaration(std::unique_ptr<Declaration> directDeclaration, SymbolTable *st, unsigned ln);
	FunctionDeclaration(std::unique_ptr<Declaration> directDeclaration, std::unique_ptr<ParameterList> parameterList, SymbolTable *st,
			unsigned ln);
	virtual ~FunctionDeclaration();

	const vector<ParameterDeclaration *> getParams() const;

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	const std::unique_ptr<Declaration> declaration;
	const std::unique_ptr<ParameterList> parameterList;
private:
	// FIXME:
	vector<ParameterDeclaration *> params;

};

} /* namespace semantic_analyzer */

#endif /* FUNCTIONDECLARATION_H_ */
