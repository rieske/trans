#include "AbstractSyntaxTree.h"

namespace ast {

AbstractSyntaxTree::AbstractSyntaxTree(std::vector<std::unique_ptr<AbstractSyntaxTreeNode> > translationUnit) :
        translationUnit { std::move(translationUnit) }
{
}

void AbstractSyntaxTree::accept(ast::AbstractSyntaxTreeVisitor& visitor) const {
    for (const auto& translationElement : translationUnit) {
        translationElement->accept(visitor);
    }
}

auto AbstractSyntaxTree::begin() const -> decltype(translationUnit.begin()) {
    return translationUnit.begin();
}

auto AbstractSyntaxTree::end() const -> decltype(translationUnit.end()) {
    return translationUnit.end();
}

void AbstractSyntaxTree::accept(parser::SyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

