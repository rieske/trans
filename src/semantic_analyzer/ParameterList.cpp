#include "ParameterList.h"

namespace semantic_analyzer {

const std::string ParameterList::ID { "<param_list>" };

ParameterList::ParameterList(ParameterList* parameterList, ParameterDeclaration* parameterDeclaration) :
		NonterminalNode(ID, { parameterList, parameterDeclaration }) {
	params = parameterList->getParams();
	params.push_back(parameterDeclaration);
	if (NULL == params[params.size() - 1]->getPlace()) {
		params.pop_back();
	}
}

ParameterList::ParameterList(ParameterDeclaration* parameterDeclaration) :
		NonterminalNode(ID, { parameterDeclaration }) {
	params.push_back(parameterDeclaration);
	if (NULL == params[params.size() - 1]->getPlace()) {
		params.pop_back();
	}
}

vector<ParameterDeclaration *> ParameterList::getParams() const {
	return params;
}

}
