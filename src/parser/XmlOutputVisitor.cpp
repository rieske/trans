#include "XmlOutputVisitor.h"

#include <string>
#include <vector>

#include "../parser/TerminalNode.h"

namespace parser {

XmlOutputVisitor::XmlOutputVisitor(std::ostream* ostream) :
		ostream { ostream } {
}

XmlOutputVisitor::~XmlOutputVisitor() {
}

void XmlOutputVisitor::visit(const parser::ParseTreeNode& node) const {
	ident();
	*ostream << "<" << node.getLabel() << ">\n";
	++identation;
	std::vector<parser::ParseTreeNode*> children = node.getChildren();
	for (const auto& node : children) {
		node->accept(*this);
	}
	--identation;
	ident();
	*ostream << "</" << node.getLabel() << ">\n";
}

void XmlOutputVisitor::visit(const parser::TerminalNode& node) const {
	ident();
	*ostream << "<terminal type='" << node.getLabel() << "' " << "value='" << node.getAttr() << "'/>\n";
}

void XmlOutputVisitor::ident() const {
	for (int i = 0; i != identation; ++i) {
		*ostream << "  ";
	}
}

} /* namespace semantic_analyzer */
