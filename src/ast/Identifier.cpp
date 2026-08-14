#include "Identifier.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

Identifier::Identifier(TerminalSymbol identifier) :
        DirectDeclarator(identifier.value, identifier.context)
{
}

void Identifier::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

type::Type Identifier::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) const {
    type::Type type = baseType;
    for (Pointer pointer : indirection) {
        // Accumulate pointer levels: `struct S **p` is pointer-to-pointer, not a
        // single pointer rebuilt from the original base each time.
        type = type::pointer(type, pointer.getQualifiers());
    }
    return type;
}

} // namespace ast

