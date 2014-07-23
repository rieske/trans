#ifndef _A_EXPRESSIONS_NODE_H_
#define _A_EXPRESSIONS_NODE_H_

#include <string>
#include <vector>

#include "AssignmentExpression.h"

namespace semantic_analyzer {

class AssignmentExpressionList: public NonterminalNode {
public:
	AssignmentExpressionList(AssignmentExpression* assignmentExpression);

	void addAssignmentExpression(AssignmentExpression* assignmentExpression);
	vector<AssignmentExpression*> getAssignmentExpressions() const;

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ID;

private:
	vector<AssignmentExpression*> assignmentExpressions;
};

}

#endif // _A_EXPRESSIONS_NODE_H_
