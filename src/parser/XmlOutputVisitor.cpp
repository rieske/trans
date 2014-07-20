#include "XmlOutputVisitor.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "TerminalNode.h"

static const std::string IDENTATION { "  " };

namespace parser {

XmlOutputVisitor::XmlOutputVisitor(std::ostream* ostream) :
		ostream { ostream } {
}

XmlOutputVisitor::~XmlOutputVisitor() {
}

void XmlOutputVisitor::visit(const ParseTreeNode& node) const {
	ident();
	*ostream << "<" << stripLabel(node.getType()) << ">\n";
	++identation;
	for (const auto& child : node.getChildren()) {
	    child->accept(*this);
	}
	--identation;
	ident();
	*ostream << "</" << stripLabel(node.getType()) << ">\n";
}

void XmlOutputVisitor::visit(const TerminalNode& node) const {
	ident();
	*ostream << "<terminal type='" << node.getType() << "' " << "value='" << node.getValue() << "'/>\n";
}

std::string XmlOutputVisitor::stripLabel(std::string label) const {
	label.erase(std::remove(label.begin(), label.end(), '<'), label.end());
	label.erase(std::remove(label.begin(), label.end(), '>'), label.end());
	return label;
}

void XmlOutputVisitor::ident() const {
	for (int i = 0; i != identation; ++i) {
		*ostream << IDENTATION;
	}
}

} /* namespace semantic_analyzer */
