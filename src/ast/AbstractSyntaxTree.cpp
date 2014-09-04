#include "AbstractSyntaxTree.h"

#include <algorithm>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../semantic_analyzer/SemanticXmlOutputVisitor.h"
#include "AbstractSyntaxTreeNode.h"

namespace ast {

AbstractSyntaxTree::AbstractSyntaxTree(std::unique_ptr<AbstractSyntaxTreeNode> top) :
        tree { std::move(top) } {
}

AbstractSyntaxTree::~AbstractSyntaxTree() {
}

void AbstractSyntaxTree::analyzeWith(semantic_analyzer::SemanticAnalyzer& semanticAnalyzer) {
    semanticAnalyzer.analyze(*tree);
}

void AbstractSyntaxTree::outputXml(std::ostream& stream) const {
    semantic_analyzer::SemanticXmlOutputVisitor xmlOutputer { &stream };
    tree->accept(xmlOutputer);
}

void AbstractSyntaxTree::outputSource(std::ostream& stream) const {

}

} /* namespace ast */
