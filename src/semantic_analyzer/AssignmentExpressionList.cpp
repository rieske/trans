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

std::ostringstream &AssignmentExpressionList::asXml(std::ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel;
	outputExprs(oss);
	oss << " code_size=\"" << code.size() << "\"";
	oss << " >\n";

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">\n";
	return oss;
}

void AssignmentExpressionList::outputExprs(std::ostringstream &oss) const {
	for (unsigned i = 0; i < exprs.size(); i++)
		exprs.at(i)->output_attr(oss, i);
}

vector<AssignmentExpression *> AssignmentExpressionList::getExprs() const {
	return exprs;
}

}
