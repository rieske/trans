#include "ParenthesizedDeclarator.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ParenthesizedDeclarator::ParenthesizedDeclarator(std::unique_ptr<Declarator> declarator) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        declarator { std::move(declarator) }
{
}

void ParenthesizedDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    // Transparent: visitors see the wrapped declarator tree (pointer + direct).
    declarator->accept(visitor);
}

type::Type ParenthesizedDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) {
    type::Type type = declarator->getFundamentalType(baseType);
    for (Pointer pointer : indirection) {
        type = type::pointer(type, pointer.getQualifiers());
    }
    return type;
}

} // namespace ast
