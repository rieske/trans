#include "XmlOutputVisitor.h"

#include <iostream>

namespace semantic_analyzer {

XmlOutputVisitor::XmlOutputVisitor(std::ostream* ostream) :
		ostream { ostream } {
}

XmlOutputVisitor::~XmlOutputVisitor() {
}

void XmlOutputVisitor::visit(const AbstractSyntaxTreeNode& node) const {
	/*ident();
	*ostream << "<" << node.getLabel() << ">\n";
	++identation;
	std::vector<AbstractSyntaxTreeNode*> children = node.getChildren();
	for (const auto& node : children) {
		node->accept(*this);
	}
	--identation;
	ident();
	*ostream << "</" << node.getLabel() << ">\n";*/
}

/*void XmlOutputVisitor::visit(const AbstractSyntaxTreeNode& node) const {
 ident();
 *ostream << "<terminal type='" << node.getLabel() << "' " << "value='" << node.getValue() << "'/>\n";
 }*/

void XmlOutputVisitor::ident() const {
	for (int i = 0; i != identation; ++i) {
		*ostream << "  ";
	}
}

} /* namespace semantic_analyzer */
