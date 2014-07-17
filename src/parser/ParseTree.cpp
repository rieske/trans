#include "ParseTree.h"

#include <algorithm>
#include <iostream>

#include "XmlOutputVisitor.h"

namespace parser {

ParseTree::ParseTree(std::unique_ptr<ParseTreeNode> top) :
		tree { std::move(top) } {
}

ParseTree::~ParseTree() {
}

void ParseTree::outputXml(std::ostream& stream) const {
	XmlOutputVisitor outputer { &stream };
	tree->accept(outputer);
}

} /* namespace parser */

