#include "AssignmentExpressionList.h"

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string AssignmentExpressionList::ID { "<a_expressions>" };

AssignmentExpressionList::AssignmentExpressionList(AssignmentExpression* assignmentExpression) {
	assignmentExpressions.push_back(assignmentExpression);
	code = assignmentExpression->getCode();
}

void AssignmentExpressionList::addAssignmentExpression(AssignmentExpression* assignmentExpression) {
	assignmentExpressions.push_back(assignmentExpression);

	vector<Quadruple *> expressionCode = assignmentExpression->getCode();
	code.insert(code.end(), expressionCode.begin(), expressionCode.end());
}

vector<AssignmentExpression*> AssignmentExpressionList::getAssignmentExpressions() const {
	return assignmentExpressions;
}

void AssignmentExpressionList::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
