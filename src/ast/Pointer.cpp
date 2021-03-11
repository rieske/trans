#include "Pointer.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

Pointer::Pointer(std::vector<type::Qualifier> qualifiers) :
        qualifiers { qualifiers }
{
}

void Pointer::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::vector<type::Qualifier> Pointer::getQualifiers() const {
    return qualifiers;
}

} // namespace ast

