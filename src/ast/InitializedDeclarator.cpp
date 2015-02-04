#include "InitializedDeclarator.h"

#include <algorithm>

#include "../translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string InitializedDeclarator::ID { "<init_declarator>" };

InitializedDeclarator::InitializedDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<Expression> initializer) :
        declarator { std::move(declarator) },
        initializer { std::move(initializer) }
{
}

void InitializedDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void InitializedDeclarator::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    declarator->accept(visitor);
    if (initializer) {
        initializer->accept(visitor);
    }
}

std::string ast::InitializedDeclarator::getName() const {
    return declarator->getName();
}

translation_unit::Context ast::InitializedDeclarator::getContext() const {
    return declarator->getContext();
}

} /* namespace ast */
