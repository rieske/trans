#include "NestedDeclarator.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

NestedDeclarator::NestedDeclarator(std::unique_ptr<Declarator> declarator) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        declarator { std::move(declarator) } {
}

void NestedDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    // No dedicated visitor method - treat as transparent for most visitors.
    // Children are visited when building types / analyzing declarators.
    visitChildren(visitor);
}

void NestedDeclarator::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    declarator->accept(visitor);
}

type::Type NestedDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) {
    type::Type type = baseType;
    for (Pointer pointer : indirection) {
        type = type::pointer(type, pointer.getQualifiers());
    }
    return declarator->getFundamentalType(type);
}

Declarator& NestedDeclarator::getDeclarator() const {
    return *declarator;
}

} // namespace ast
