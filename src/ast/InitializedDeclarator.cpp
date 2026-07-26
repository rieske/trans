#include "InitializedDeclarator.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

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

Expression* InitializedDeclarator::getInitializer() const {
    return initializer.get();
}

symbols::ValueEntry* InitializedDeclarator::getInitializerHolder(symbols::AnnotationStore& store) const {
    return initializer->getResultSymbol(store);
}

translation_unit::Context ast::InitializedDeclarator::getContext() const {
    return declarator->getContext();
}

void InitializedDeclarator::setHolder(symbols::ValueEntry holder) {
    this->holder = std::make_unique<symbols::ValueEntry>(holder);
}

symbols::ValueEntry* InitializedDeclarator::getHolder() const {
    if (!holder) {
        throw std::runtime_error { "InitializedDeclarator::getHolder() == nullptr" };
    }
    return holder.get();
}

type::Type InitializedDeclarator::getFundamentalType(const type::Type& baseType) {
    return declarator->getFundamentalType(baseType);
}

} // namespace ast
