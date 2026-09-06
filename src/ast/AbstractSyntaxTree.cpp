#include "AbstractSyntaxTree.h"

namespace ast {

AbstractSyntaxTree::AbstractSyntaxTree(std::vector<ExternalDeclaration> translationUnit) :
        translationUnit { std::move(translationUnit) }
{
}

void AbstractSyntaxTree::accept(ast::AbstractSyntaxTreeVisitor& visitor) const {
    for (const auto& translationElement : translationUnit) {
        translationElement.accept(visitor);
    }
}

} // namespace ast

