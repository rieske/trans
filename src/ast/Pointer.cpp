#include "Pointer.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string Pointer::ID { "<pointer>" };

Pointer::Pointer(std::vector<type::Qualifier> qualifiers) :
        qualifiers { qualifiers }
{
}

Pointer::Pointer(const Pointer& rhs) :
        AbstractSyntaxTreeNode { },
        qualifiers { rhs.qualifiers }
{
}

Pointer::Pointer(Pointer&& rhs) :
        qualifiers { std::move(rhs.qualifiers) }
{
}

Pointer& Pointer::operator=(const Pointer& rhs) {
    if (&rhs == this) {
        return *this;
    }
    qualifiers = rhs.qualifiers;
    return *this;
}

Pointer& Pointer::operator=(Pointer&& rhs) {
    qualifiers = std::move(rhs.qualifiers);
    return *this;
}

void Pointer::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::vector<type::Qualifier> Pointer::getQualifiers() const {
    return qualifiers;
}

} // namespace ast

