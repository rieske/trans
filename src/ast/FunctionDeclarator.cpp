#include "FunctionDeclarator.h"

#include <algorithm>

#include "translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "FormalArgument.h"

namespace ast {

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<Declarator> declarator) :
        FunctionDeclarator { std::move(declarator), std::make_unique<std::vector<std::unique_ptr<FormalArgument>>>()}
    {
    }

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<std::vector<std::unique_ptr<FormalArgument>>> formalArguments) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        formalArguments { std::move(formalArguments) }
{
}

FunctionDeclarator::~FunctionDeclarator() {
}

void FunctionDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace ast */
