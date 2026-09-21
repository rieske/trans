#include "Pointer.h"

#include "AbstractSyntaxTreeVisitor.h"

#include <utility>

namespace ast {

Pointer::Pointer(std::vector<type::Qualifier> qualifiers) :
        qualifiers { std::move(qualifiers) }
{
}

void Pointer::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::vector<type::Qualifier> Pointer::getQualifiers() const {
    return qualifiers;
}

} // namespace ast

