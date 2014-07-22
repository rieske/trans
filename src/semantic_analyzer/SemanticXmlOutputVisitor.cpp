#include "SemanticXmlOutputVisitor.h"

#include <algorithm>
#include <iterator>

#include "AssignmentExpression.h"
#include "AssignmentExpressionList.h"
#include "ParameterDeclaration.h"
#include "ParameterList.h"

static const std::string IDENTATION { "  " };

namespace semantic_analyzer {

SemanticXmlOutputVisitor::SemanticXmlOutputVisitor(std::ostream* outputStream) :
		outputStream { outputStream } {
}

SemanticXmlOutputVisitor::~SemanticXmlOutputVisitor() {
}

void SemanticXmlOutputVisitor::visit(const AbstractSyntaxTreeNode& node) const {

}

void SemanticXmlOutputVisitor::visit(const NonterminalNode& node) const {
	openXmlNode(node.typeId());
	for (const auto& child : node.getChildren()) {
		child->accept(*this);
	}
	closeXmlNode(node.typeId());
}

void SemanticXmlOutputVisitor::visit(const ParameterList& parameterList) const {
	openXmlNode(parameterList.ID);
	for (const auto& parameter : parameterList.getDeclaredParameters()) {
		parameter->accept(*this);
	}
	closeXmlNode(parameterList.ID);
}

void SemanticXmlOutputVisitor::visit(const AssignmentExpressionList& assignmentExpressions) const {
	openXmlNode(assignmentExpressions.ID);
	for (const auto& parameter : assignmentExpressions.getAssignmentExpressions()) {
		parameter->accept(*this);
	}
	closeXmlNode(assignmentExpressions.ID);
}

void SemanticXmlOutputVisitor::openXmlNode(const std::string& nodeName) const {
	ident();
	*outputStream << "<" << stripLabel(nodeName) << ">\n";
	++identation;
}

void SemanticXmlOutputVisitor::closeXmlNode(const std::string& nodeName) const {
	--identation;
	ident();
	*outputStream << "</" << stripLabel(nodeName) << ">\n";
}

std::string SemanticXmlOutputVisitor::stripLabel(std::string label) const {
	label.erase(std::remove(label.begin(), label.end(), '<'), label.end());
	label.erase(std::remove(label.begin(), label.end(), '>'), label.end());
	return label;
}

void SemanticXmlOutputVisitor::ident() const {
	for (int i = 0; i != identation; ++i) {
		*outputStream << IDENTATION;
	}
}

} /* namespace semantic_analyzer */
