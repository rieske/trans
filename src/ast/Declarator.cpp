#include "Declarator.h"

#include <algorithm>
#include <stdexcept>

#include "../translation_unit/Context.h"
#include "DirectDeclarator.h"

namespace ast {

const std::string Declarator::ID = "<declarator>";

Declarator::Declarator(std::unique_ptr<DirectDeclarator> declarator, std::unique_ptr<Pointer> pointer) :
        declarator { std::move(declarator) },
        pointer { std::move(pointer) }
{
}

void Declarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    throw std::runtime_error { "Declarator::accept is not implemented" };
}

std::string Declarator::getName() const {
    return declarator->getName();
}

translation_unit::Context Declarator::getContext() const {
    return declarator->getContext();
}

}

