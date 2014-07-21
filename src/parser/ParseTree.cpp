#include "ParseTree.h"

#include <algorithm>
#include <iostream>

#include "ParseTreeNode.h"
#include "ParseTreeToSourceConverter.h"
#include "XmlOutputVisitor.h"

namespace parser {

ParseTree::ParseTree(std::unique_ptr<ParseTreeNode> top) :
        tree { std::move(top) } {
}

ParseTree::~ParseTree() {
}

void ParseTree::outputXml(std::ostream& stream) const {
    XmlOutputVisitor toXml { &stream };
    tree->accept(toXml);
}

void ParseTree::outputSource(std::ostream& stream) const {
    ParseTreeToSourceConverter toSource { &stream };
    tree->accept(toSource);
}

} /* namespace parser */

