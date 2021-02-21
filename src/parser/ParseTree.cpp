#include "ParseTree.h"

#include "ParseTreeToSourceConverter.h"
#include "XmlOutputVisitor.h"

namespace parser {

ParseTree::ParseTree(std::unique_ptr<ParseTreeNode> top) :
        tree { std::move(top) }
{
}

ParseTree::~ParseTree() = default;

void ParseTree::accept(ParseTreeNodeVisitor& visitor) const {
    tree->accept(visitor);
}

void ParseTree::accept(parser::SyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace parser

