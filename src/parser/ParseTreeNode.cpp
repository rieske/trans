#include "ParseTreeNode.h"

#include "ParseTreeNodeVisitor.h"

namespace parser {

ParseTreeNode::ParseTreeNode(std::string type, std::vector<std::unique_ptr<ParseTreeNode>> children) :
		type { type },
		subtrees { std::move(children) } {
}

std::string ParseTreeNode::getType() const {
	return type;
}

const std::vector<std::unique_ptr<ParseTreeNode>>& ParseTreeNode::getChildren() const {
	return subtrees;
}

void ParseTreeNode::accept(const ParseTreeNodeVisitor& visitor) const {
	visitor.visit(*this);
}

} // namespace parser

