#include "FunctionDeclarator.h"

#include <algorithm>
#include <string>

#include "AbstractSyntaxTreeVisitor.h"
#include "ParameterList.h"

namespace ast {

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<Declarator> declarator) :
        FunctionDeclarator(std::move(declarator), std::unique_ptr<ParameterList> { new ParameterList() }) {
}

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<Declarator> declarator,
        std::unique_ptr<ParameterList> formalArguments) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        formalArguments { std::move(formalArguments) } {
}

FunctionDeclarator::~FunctionDeclarator() {
}

void FunctionDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace ast */
