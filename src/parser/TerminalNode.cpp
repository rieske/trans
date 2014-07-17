#include "TerminalNode.h"

#include <iostream>
#include <string>
#include <sstream>

#include "ParseTreeNodeVisitor.h"

namespace parser {

TerminalNode::TerminalNode(string label, string lexeme) :
		ParseTreeNode(label) {
	value = lexeme;
}

string TerminalNode::getAttr() const {
	return value;
}

std::ostringstream &TerminalNode::asXml(std::ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "<terminal label='" << xmlEncode(label) << "' " << "value='" << xmlEncode(value) << "' " << " />\n";
	return oss;
}

void TerminalNode::accept(const ParseTreeNodeVisitor& visitor) const {
	visitor.visit(*this);
}

}
