#include "Declarator.h"

#include <algorithm>

#include "../translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string Declarator::ID = "<declarator>";

Declarator::Declarator(std::unique_ptr<DirectDeclarator> declarator, std::unique_ptr<Pointer> pointer) :
        declarator { std::move(declarator) },
        pointer { std::move(pointer) }
{
}

void Declarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void Declarator::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    if (pointer) {
        pointer->accept(visitor);
    }
    declarator->accept(visitor);
}

std::string Declarator::getName() const {
    return declarator->getName();
}

translation_unit::Context Declarator::getContext() const {
    return declarator->getContext();
}

}

