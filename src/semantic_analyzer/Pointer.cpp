#include "Pointer.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string Pointer::ID { "<ptr>" };

Pointer::Pointer() {
}

void Pointer::dereference() {
    ++dereferenceCount;
}

int Pointer::getDereferenceCount() const {
    return dereferenceCount;
}

void Pointer::accept(const AbstractSyntaxTreeVisitor& visitor) const {
    visitor.visit(*this);
}

}
