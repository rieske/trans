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

type::Type NestedDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) const {
    // Transparent for type composition: feed outer pointers into the nested declarator
    // so `T *(a[N])` is array-of-pointers, not pointer-to-array.
    return declarator->getFundamentalType(std::move(indirection), baseType);
}

Declarator& NestedDeclarator::getDeclarator() const {
    return *declarator;
}

void NestedDeclarator::forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn) {
    declarator->forEachArrayDeclarator(fn);
}

const FunctionDeclarator* NestedDeclarator::innermostFunctionDeclarator() const {
    return declarator->innermostFunctionDeclarator();
}

void NestedDeclarator::foldArrayBoundSizeofs(const std::function<void(Expression*)>& foldSizeof) {
    declarator->foldArrayBoundSizeofs(foldSizeof);
}

bool NestedDeclarator::hasArrayDeclarator() const {
    return declarator->hasArrayDeclarator();
}

} // namespace ast
