#ifndef _PARAM_LIST_NODE_H_
#define _PARAM_LIST_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "ParameterDeclaration.h"

namespace semantic_analyzer {

class ParameterList: public NonterminalNode {
public:
	ParameterList(ParameterList* parameterList, ParameterDeclaration* parameterDeclaration);
	ParameterList(ParameterDeclaration* parameterDeclaration);

	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

	vector<ParameterDeclaration *> getParams() const;

	void outputParams(ostringstream &oss) const;

	static const std::string ID;

private:
	vector<ParameterDeclaration *> params;
};

}

#endif // _PARAM_LIST_NODE_H_
