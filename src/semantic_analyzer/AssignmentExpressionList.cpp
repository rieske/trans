#include "AssignmentExpressionList.h"

#include <iterator>
#include <sstream>

#include "../code_generator/quadruple.h"

namespace semantic_analyzer {

const std::string AssignmentExpressionList::ID { "<a_expressions>" };

AssignmentExpressionList::AssignmentExpressionList(AssignmentExpressionList* exprsNode, AssignmentExpression* exprNode) :
		NonterminalNode(ID, { exprsNode, exprNode }) {
	exprs = exprsNode->getExprs();
	code = exprsNode->getCode();
	exprs.push_back(exprNode);
	vector<Quadruple *> more_code = exprNode->getCode();
	code.insert(code.end(), more_code.begin(), more_code.end());
}

AssignmentExpressionList::AssignmentExpressionList(AssignmentExpression* exprNode) :
		NonterminalNode(ID, { exprNode }) {
	exprs.push_back(exprNode);
	code = exprNode->getCode();
}

vector<AssignmentExpression *> AssignmentExpressionList::getExprs() const {
	return exprs;
}

}
