#include "Identifier.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "types/PointerType.h"

namespace ast {

Identifier::Identifier(TerminalSymbol identifier) :
        DirectDeclarator(identifier.value, identifier.context)
{
}

void Identifier::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

type::Type Identifier::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) {
    type::Type type = baseType;
    for (Pointer pointer : indirection) {
        type = type::pointer(baseType, pointer.getQualifiers());
    }
    return type;
}

} // namespace ast

