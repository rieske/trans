#include "InitializedDeclarator.h"

#include <algorithm>
#include <stdexcept>

#include "../semantic_analyzer/ValueEntry.h"
#include "../translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "types/FundamentalType.h"
#include "DirectDeclarator.h"

namespace ast {

const std::string InitializedDeclarator::ID { "<init_declarator>" };

InitializedDeclarator::InitializedDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<Expression> initializer) :
        declarator { std::move(declarator) },
        initializer { std::move(initializer) }
{
}

void InitializedDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void InitializedDeclarator::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    declarator->accept(visitor);
    if (initializer) {
        initializer->accept(visitor);
    }
}

std::string ast::InitializedDeclarator::getName() const {
    return declarator->getName();
}

bool InitializedDeclarator::hasInitializer() const {
    return !!initializer;
}

semantic_analyzer::ValueEntry* InitializedDeclarator::getInitializerHolder() const {
    return initializer->getResultSymbol();
}

translation_unit::Context ast::InitializedDeclarator::getContext() const {
    return declarator->getContext();
}

void InitializedDeclarator::setHolder(semantic_analyzer::ValueEntry holder) {
    this->holder = std::make_unique<semantic_analyzer::ValueEntry>(holder);
}

semantic_analyzer::ValueEntry* InitializedDeclarator::getHolder() const {
    if (!holder) {
        throw std::runtime_error { "InitializedDeclarator::getHolder() == nullptr" };
    }
    return holder.get();
}

std::unique_ptr<FundamentalType> InitializedDeclarator::getFundamentalType(const FundamentalType& baseType) {
    return declarator->getFundamentalType(baseType);
}

} /* namespace ast */

