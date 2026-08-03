#include "InitializedDeclarator.h"
#include <stdexcept>
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

InitializedDeclarator::InitializedDeclarator(std::unique_ptr<Declarator> declarator,
        std::unique_ptr<Expression> initializer)
    : declarator{std::move(declarator)}, initializer{std::move(initializer)} {}

void InitializedDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }

void InitializedDeclarator::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    declarator->accept(visitor);
    if (initializer) {
        initializer->accept(visitor);
    }
}

std::string InitializedDeclarator::getName() const { return declarator->getName(); }
Declarator* InitializedDeclarator::getDeclarator() const { return declarator.get(); }
bool InitializedDeclarator::hasInitializer() const { return !!initializer; }
Expression* InitializedDeclarator::getInitializer() const { return initializer.get(); }

void InitializedDeclarator::setInitializer(std::unique_ptr<Expression> init) {
    initializer = std::move(init);
}

translation_unit::Context InitializedDeclarator::getContext() const {
    return declarator->getContext();
}



type::Type InitializedDeclarator::getFundamentalType(const type::Type& baseType) const {
    return declarator->getFundamentalType(baseType);
}

void InitializedDeclarator::forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn) {
    declarator->forEachArrayDeclarator(fn);
}

} // namespace ast
