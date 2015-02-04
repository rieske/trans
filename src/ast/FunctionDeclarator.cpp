#include "FunctionDeclarator.h"

#include <algorithm>

#include "translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "FormalArgument.h"
#include "Declarator.h"

namespace ast {

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        formalArguments { std::move(formalArguments) }
{
}

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator, FormalArguments formalArguments) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        formalArguments { std::move(formalArguments) }
{
}

void FunctionDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void FunctionDeclarator::visitFormalArguments(AbstractSyntaxTreeVisitor& visitor) {
    for (auto& argument : formalArguments) {
        argument.accept(visitor);
    }
}

const FormalArguments& ast::FunctionDeclarator::getFormalArguments() const {
    return formalArguments;
}

} /* namespace ast */
