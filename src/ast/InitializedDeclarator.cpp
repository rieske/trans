#include "InitializedDeclarator.h"

#include <algorithm>

#include "../code_generator/ValueEntry.h"
#include "../translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"

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

int InitializedDeclarator::getDereferenceCount() const {
    return declarator->getDereferenceCount();
}

bool InitializedDeclarator::hasInitializer() const {
    return !!initializer;
}

code_generator::ValueEntry* InitializedDeclarator::getInitializerHolder() const {
    return initializer->getResultSymbol();
}

translation_unit::Context ast::InitializedDeclarator::getContext() const {
    return declarator->getContext();
}

void InitializedDeclarator::setHolder(code_generator::ValueEntry holder) {
    this->holder = std::make_unique<code_generator::ValueEntry>(holder);
}

code_generator::ValueEntry* InitializedDeclarator::getHolder() const {
    if (!holder) {
        throw std::runtime_error { "InitializedDeclarator::getHolder() == nullptr" };
    }
    return holder.get();
}

} /* namespace ast */
