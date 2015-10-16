#include "ast/Pointer.h"

#include <algorithm>

#include "ast/AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string Pointer::ID { "<pointer>" };

Pointer::Pointer(std::vector<TypeQualifier> qualifiers) :
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

std::set<TypeQualifier> Pointer::getQualifiers() const {
    std::set<TypeQualifier> qualifierSet;
    for (auto qualifier : qualifiers) {
        qualifierSet.insert(qualifier);
    }
    return qualifierSet;
}

}

