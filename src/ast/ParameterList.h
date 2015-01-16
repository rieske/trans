#ifndef _PARAM_LIST_NODE_H_
#define _PARAM_LIST_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "AbstractSyntaxTreeNode.h"
#include "FormalArgument.h"

namespace ast {

class ParameterList: public AbstractSyntaxTreeNode {
public:
	ParameterList();
	ParameterList(std::unique_ptr<FormalArgument> parameterDeclaration);

	void addParameterDeclaration(std::unique_ptr<FormalArgument> parameterDeclaration);
	const std::vector<std::unique_ptr<FormalArgument>>& getDeclaredParameters() const;

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

private:
	std::vector<std::unique_ptr<FormalArgument>> declaredParameters;
};

}

#endif // _PARAM_LIST_NODE_H_
