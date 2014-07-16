#include "ParameterList.h"

#include <sstream>


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

std::ostringstream &ParameterList::asXml(std::ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel;
	outputParams(oss);
	oss << " >\n";

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">\n";
	return oss;
}

void ParameterList::outputParams(std::ostringstream &oss) const {
	for (unsigned i = 0; i < params.size(); i++)
		params.at(i)->output_attr(oss, i);
}

vector<ParameterDeclaration *> ParameterList::getParams() const {
	return params;
}

}
