#include "ast/Declarator.h"

#include <algorithm>

#include "translation_unit/Context.h"
#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/DirectDeclarator.h"

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

std::unique_ptr<FundamentalType> ast::Declarator::getFundamentalType(const FundamentalType& baseType) {
    return declarator->getFundamentalType(indirection, baseType);
}

}
