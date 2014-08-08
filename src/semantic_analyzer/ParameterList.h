#ifndef _PARAM_LIST_NODE_H_
#define _PARAM_LIST_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "NonterminalNode.h"
#include "ParameterDeclaration.h"

namespace semantic_analyzer {

class ParameterList: public NonterminalNode {
public:
	ParameterList();
	ParameterList(std::unique_ptr<ParameterDeclaration> parameterDeclaration);

	void addParameterDeclaration(std::unique_ptr<ParameterDeclaration> parameterDeclaration);
	const std::vector<std::unique_ptr<ParameterDeclaration>>& getDeclaredParameters() const;

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

private:
	vector<std::unique_ptr<ParameterDeclaration>> declaredParameters;
};

}

#endif // _PARAM_LIST_NODE_H_
