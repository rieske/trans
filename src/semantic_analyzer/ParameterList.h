#ifndef _PARAM_LIST_NODE_H_
#define _PARAM_LIST_NODE_H_

#include <string>
#include <vector>

#include "ParameterDeclaration.h"

namespace semantic_analyzer {

class ParameterList: public NonterminalNode {
public:
	ParameterList(ParameterDeclaration* parameterDeclaration);

	void addParameterDeclaration(ParameterDeclaration* parameterDeclaration);
	vector<ParameterDeclaration*> getDeclaredParameters() const;

	void accept(const AbstractSyntaxTreeVisitor& visitor) const;

	static const std::string ID;

private:
	vector<ParameterDeclaration*> declaredParameters;
};

}

#endif // _PARAM_LIST_NODE_H_
