#include "parser/TerminalNode.h"

#include <string>

#include "parser/ParseTreeNodeVisitor.h"

using std::string;

namespace parser {

TerminalNode::TerminalNode(string type, string value) :
		ParseTreeNode(type, { }),
		value { value } {
}

string TerminalNode::getValue() const {
	return value;
}

void TerminalNode::accept(const ParseTreeNodeVisitor& visitor) const {
	visitor.visit(*this);
}

}
