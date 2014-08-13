#include "AbstractSyntaxTree.h"

#include <algorithm>

#include "AbstractSyntaxTreeNode.h"
#include "SemanticAnalyzer.h"
#include "SemanticXmlOutputVisitor.h"

namespace semantic_analyzer {

AbstractSyntaxTree::AbstractSyntaxTree(std::unique_ptr<AbstractSyntaxTreeNode> top) :
        tree { std::move(top) } {
}

AbstractSyntaxTree::~AbstractSyntaxTree() {
}

void AbstractSyntaxTree::analyzeWith(semantic_analyzer::SemanticAnalyzer& semanticAnalyzer) {
    semanticAnalyzer.analyze(*tree);
}

void AbstractSyntaxTree::outputXml(std::ostream& stream) const {
    SemanticXmlOutputVisitor xmlOutputer { &stream };
    tree->accept(xmlOutputer);
}

void AbstractSyntaxTree::outputSource(std::ostream& stream) const {

}

} /* namespace semantic_analyzer */
