#ifndef _A_EXPRESSIONS_NODE_H_
#define _A_EXPRESSIONS_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "AssignmentExpression.h"

namespace semantic_analyzer {

class AssignmentExpressionList: public NonterminalNode {
public:
	AssignmentExpressionList(AssignmentExpressionList* exprsNode, TerminalNode* comma, AssignmentExpression* exprNode);
	AssignmentExpressionList(AssignmentExpression* exprNode);

	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

	vector<AssignmentExpression *> getExprs() const;

	void outputExprs(ostringstream &oss) const;

	static const std::string ID;

private:
	vector<AssignmentExpression *> exprs;
};

}

#endif // _A_EXPRESSIONS_NODE_H_
