#include "Declarator.h"

#include <algorithm>

#include "translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "DirectDeclarator.h"

namespace ast {

const std::string Declarator::ID = "<declarator>";

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
    return declarator->getFundamentalType(indirection, baseType);
}

} // namespace ast
