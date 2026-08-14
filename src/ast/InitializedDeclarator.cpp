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
    visitDeclarator(visitor);
    visitInitializer(visitor);
}

void InitializedDeclarator::visitDeclarator(AbstractSyntaxTreeVisitor& visitor) {
    declarator->accept(visitor);
}

void InitializedDeclarator::visitInitializer(AbstractSyntaxTreeVisitor& visitor) {
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

void InitializedDeclarator::setInitializer(std::unique_ptr<Expression> init) {
    initializer = std::move(init);
}

symbols::ValueEntry* InitializedDeclarator::getInitializerHolder(symbols::AnnotationStore& store) const {
    return initializer->getResultSymbol(store);
}

translation_unit::Context ast::InitializedDeclarator::getContext() const {
    return declarator->getContext();
}

void InitializedDeclarator::setHolder(symbols::AnnotationStore& store, symbols::ValueEntry holder) {
    store.setHolder(this, std::move(holder));
}

symbols::ValueEntry* InitializedDeclarator::getHolder(symbols::AnnotationStore& store) const {
    // Soft probe (nullptr when missing); CG asserts after successful SA.
    return store.holder(this);
}

type::Type InitializedDeclarator::getFundamentalType(const type::Type& baseType) const {
    return declarator->getFundamentalType(baseType);
}

void InitializedDeclarator::forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn) {
    declarator->forEachArrayDeclarator(fn);
}

} // namespace ast
