#include "ParseTree.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "ParseTreeNode.h"
#include "ParseTreeToSourceConverter.h"
#include "XmlOutputVisitor.h"

namespace parser {

ParseTree::ParseTree(std::unique_ptr<ParseTreeNode> top) :
        tree { std::move(top) }
{
}

void ParseTree::outputXml(std::ostream& stream) const {
    XmlOutputVisitor toXml { &stream };
    tree->accept(toXml);
}

void ParseTree::accept(parser::SyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void ParseTree::outputSource(std::ostream& stream) const {
    ParseTreeToSourceConverter toSource { &stream };
    tree->accept(toSource);
}

} /* namespace parser */

