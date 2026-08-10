#include "Declaration.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

Declaration::Declaration(DeclarationSpecifiers declarationSpecifiers, std::vector<std::unique_ptr<InitializedDeclarator>> declarators) :
        declarationSpecifiers { declarationSpecifiers },
        declarators { std::move(declarators) }
{
}

void Declaration::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void Declaration::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    visitSpecifiers(visitor);
    for (auto& declarator : declarators) {
        declarator->accept(visitor);
    }
}

void Declaration::visitSpecifiers(AbstractSyntaxTreeVisitor& visitor) {
    declarationSpecifiers.accept(visitor);
}

const DeclarationSpecifiers& ast::Declaration::getDeclarationSpecifiers() const {
    return declarationSpecifiers;
}

const std::vector<std::unique_ptr<InitializedDeclarator> >& ast::Declaration::getDeclarators() const {
    return declarators;
}

} // namespace ast
