#include "ParameterList.h"

#include "FormalArgument.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string ParameterList::ID { "<param_list>" };

ParameterList::ParameterList() {
}

ParameterList::ParameterList(std::unique_ptr<FormalArgument> parameterDeclaration) {
	declaredParameters.push_back(std::move(parameterDeclaration));
}

void ParameterList::addParameterDeclaration(std::unique_ptr<FormalArgument> parameterDeclaration) {
	declaredParameters.push_back(std::move(parameterDeclaration));
}

const std::vector<std::unique_ptr<FormalArgument>>& ParameterList::getDeclaredParameters() const {
	return declaredParameters;
}

void ParameterList::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

}

