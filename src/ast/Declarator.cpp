#include "Declarator.h"

#include <algorithm>

#include "translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "DirectDeclarator.h"

namespace ast {

Declarator::Declarator(std::unique_ptr<DirectDeclarator> declarator, std::vector<Pointer> indirection) :
        declarator { std::move(declarator) },
        indirection { std::move(indirection) }
{
}

void Declarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void Declarator::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    for (auto& pointer : indirection) {
        pointer.accept(visitor);
    }
    declarator->accept(visitor);
}

std::string Declarator::getName() const {
    return declarator->getName();
}

translation_unit::Context Declarator::getContext() const {
    return declarator->getContext();
}

type::Type ast::Declarator::getFundamentalType(const type::Type& baseType) {
    return getFundamentalType({}, baseType);
}

type::Type ast::Declarator::getFundamentalType(std::vector<Pointer> outerIndirection, const type::Type& baseType) {
    // Outer then own pointers so array direct-declarators see them as element pointers
    // (`T *(a[N])` == `T *a[N]`).
    std::vector<Pointer> combined = std::move(outerIndirection);
    combined.insert(combined.end(), indirection.begin(), indirection.end());
    return declarator->getFundamentalType(combined, baseType);
}

void Declarator::forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn) {
    declarator->forEachArrayDeclarator(fn);
}

} // namespace ast
