#include "Pointer.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string Pointer::ID { "<ptr>" };

Pointer::Pointer() :
        extendedType { "p" } {
}

void Pointer::dereference() {
    extendedType += "p";
}

std::string Pointer::getExtendedType() const {
    return extendedType;
}

void Pointer::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
