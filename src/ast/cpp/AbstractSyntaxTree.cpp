#include "ast/AbstractSyntaxTree.h"

#include <algorithm>

#include "semantic_analyzer/SemanticXmlOutputVisitor.h"
#include "ast/AbstractSyntaxTreeNode.h"

namespace ast {

AbstractSyntaxTree::AbstractSyntaxTree(std::vector<std::unique_ptr<AbstractSyntaxTreeNode> > translationUnit) :
        translationUnit { std::move(translationUnit) }
{
}

void AbstractSyntaxTree::outputXml(std::ostream& stream) const {
    semantic_analyzer::SemanticXmlOutputVisitor xmlOutputer { &stream };
    for (const auto& translationElement : translationUnit) {
        translationElement->accept(xmlOutputer);
    }
}

const auto AbstractSyntaxTree::begin() const -> decltype(translationUnit.begin()) {
    return translationUnit.begin();
}

const auto AbstractSyntaxTree::end() const -> decltype(translationUnit.end()) {
    return translationUnit.end();
}

void AbstractSyntaxTree::accept(parser::SyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void AbstractSyntaxTree::outputSource(std::ostream& stream) const {

}

} /* namespace ast */
