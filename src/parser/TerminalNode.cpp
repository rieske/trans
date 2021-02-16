#include "parser/TerminalNode.h"

#include "ParseTreeNodeVisitor.h"

namespace parser {

TerminalNode::TerminalNode(std::string type, std::string value) :
		ParseTreeNode(type, { }),
		value { value } {
}

std::string TerminalNode::getValue() const {
	return value;
}

void TerminalNode::accept(const ParseTreeNodeVisitor& visitor) const {
	visitor.visit(*this);
}

} // namespace parser

