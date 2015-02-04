#include "AbstractSyntaxTree.h"

#include <algorithm>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../semantic_analyzer/SemanticXmlOutputVisitor.h"
#include "AbstractSyntaxTreeNode.h"

namespace ast {

AbstractSyntaxTree::AbstractSyntaxTree(std::vector<std::unique_ptr<AbstractSyntaxTreeNode> > translationUnit) :
        translationUnit { std::move(translationUnit) }
{
}

void AbstractSyntaxTree::analyzeWith(semantic_analyzer::SemanticAnalyzer& semanticAnalyzer) {
    semanticAnalyzer.analyze(translationUnit);
}

void AbstractSyntaxTree::outputXml(std::ostream& stream) const {
    semantic_analyzer::SemanticXmlOutputVisitor xmlOutputer { &stream };
    for (const auto& translationElement : translationUnit) {
        translationElement->accept(xmlOutputer);
    }
}

void AbstractSyntaxTree::outputSource(std::ostream& stream) const {

}

} /* namespace ast */
