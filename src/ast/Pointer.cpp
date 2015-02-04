#include "Pointer.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string Pointer::ID { "<pointer>" };

Pointer::Pointer(Pointer* pointer, std::vector<TypeQualifier> qualifiers) :
        pointer { pointer },
        qualifiers { qualifiers }
{
}

Pointer::Pointer(std::vector<TypeQualifier> qualifiers) :
        Pointer { nullptr, qualifiers }
{
}

Pointer::Pointer(const Pointer& rhs) :
        AbstractSyntaxTreeNode { },
        pointer { (rhs.pointer ? new Pointer(*rhs.pointer) : nullptr) },
        qualifiers { rhs.qualifiers }
{
}

Pointer::Pointer(Pointer&& rhs) :
        pointer { rhs.pointer ? std::move(rhs.pointer) : nullptr },
        qualifiers { std::move(rhs.qualifiers) }
{
}

Pointer& Pointer::operator=(const Pointer& rhs) {
    if (&rhs == this) {
        return *this;
    }
    if (rhs.pointer) {
        pointer = std::make_unique<Pointer>(*rhs.pointer);
    } else {
        pointer.reset();
    }
    qualifiers = rhs.qualifiers;
    return *this;
}

Pointer& Pointer::operator=(Pointer&& rhs) {
    if (rhs.pointer) {
        pointer = std::move(rhs.pointer);
    }
    qualifiers = std::move(rhs.qualifiers);
    return *this;
}

int Pointer::getDereferenceCount() const {
    return 1 + (pointer ? pointer->getDereferenceCount() : 0);
}

void Pointer::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
