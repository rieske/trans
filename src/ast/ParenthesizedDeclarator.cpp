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

type::Type ParenthesizedDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) const {
    // Transparent for type composition: feed outer pointers into the nested declarator
    // so `T *(a[N])` is array-of-pointers, not pointer-to-array.
    return declarator->getFundamentalType(std::move(indirection), baseType);
}

void ParenthesizedDeclarator::forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn) {
    declarator->forEachArrayDeclarator(fn);
}

const FunctionDeclarator* ParenthesizedDeclarator::innermostFunctionDeclarator() const {
    return declarator->innermostFunctionDeclarator();
}

} // namespace ast
