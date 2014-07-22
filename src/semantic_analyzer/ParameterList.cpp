#include "ParameterList.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string ParameterList::ID { "<param_list>" };

ParameterList::ParameterList(ParameterDeclaration* parameterDeclaration) {
	declaredParameters.push_back(parameterDeclaration);
}

void ParameterList::addParameterDeclaration(ParameterDeclaration* parameterDeclaration) {
	declaredParameters.push_back(parameterDeclaration);
}

vector<ParameterDeclaration *> ParameterList::getDeclaredParameters() const {
	return declaredParameters;
}

void ParameterList::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}

