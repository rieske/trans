#include "FunctionDeclarator.h"

#include <algorithm>

#include "translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "FormalArgument.h"

namespace ast {

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<Declarator> declarator) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        formalArguments { std::move(formalArguments) }
{
}

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<Declarator> declarator, FormalArguments formalArguments) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        formalArguments { std::move(formalArguments) }
{
}

FunctionDeclarator::~FunctionDeclarator() {
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
