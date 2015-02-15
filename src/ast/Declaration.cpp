#include "Declaration.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "InitializedDeclarator.h"
#include "DirectDeclarator.h"

namespace ast {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(DeclarationSpecifiers declarationSpecifiers, std::vector<std::unique_ptr<InitializedDeclarator>> declarators) :
        declarationSpecifiers { declarationSpecifiers },
        declarators { std::move(declarators) }
{
}

void Declaration::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void Declaration::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    declarationSpecifiers.accept(visitor);
    for (auto& declarator : declarators) {
        declarator->accept(visitor);
    }
}

DeclarationSpecifiers ast::Declaration::getDeclarationSpecifiers() const {
    return declarationSpecifiers;
}

const std::vector<std::unique_ptr<InitializedDeclarator> >& ast::Declaration::getDeclarators() const {
    return declarators;
}

} /* namespace ast */
