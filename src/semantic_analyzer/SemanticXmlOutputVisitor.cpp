#include "SemanticXmlOutputVisitor.h"

#include <algorithm>
#include <iterator>

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
	openXmlNode(node);
	for (const auto& child : node.getChildren()) {
		child->accept(*this);
	}
	closeXmlNode(node);
}

std::string SemanticXmlOutputVisitor::stripLabel(std::string label) const {
	label.erase(std::remove(label.begin(), label.end(), '<'), label.end());
	label.erase(std::remove(label.begin(), label.end(), '>'), label.end());
	return label;
}

void SemanticXmlOutputVisitor::visit(const ParameterList& parameterList) const {
	openXmlNode(parameterList);
	for (const auto& parameter : parameterList.getDeclaredParameters()) {
		parameter->accept(*this);
	}
	closeXmlNode(parameterList);
}

void SemanticXmlOutputVisitor::openXmlNode(const NonterminalNode& node) const {
	ident();
	*outputStream << "<" << stripLabel(node.getType()) << ">\n";
	++identation;
}

void SemanticXmlOutputVisitor::closeXmlNode(const NonterminalNode& node) const {
	--identation;
	ident();
	*outputStream << "</" << stripLabel(node.getType()) << ">\n";
}

void SemanticXmlOutputVisitor::ident() const {
	for (int i = 0; i != identation; ++i) {
		*outputStream << IDENTATION;
	}
}

} /* namespace semantic_analyzer */
