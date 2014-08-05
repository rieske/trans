#include "ParseTree.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "ParseTreeNode.h"
#include "ParseTreeToSourceConverter.h"
#include "XmlOutputVisitor.h"

namespace semantic_analyzer {
class SemanticAnalyzer;
} /* namespace semantic_analyzer */

namespace parser {

ParseTree::ParseTree(std::unique_ptr<ParseTreeNode> top) :
        tree { std::move(top) } {
}

ParseTree::~ParseTree() {
}

void ParseTree::analyzeWith(semantic_analyzer::SemanticAnalyzer&) {
    throw std::runtime_error { "semantic analysis will not be performed on parse tree" };
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

