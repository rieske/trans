#include "FunctionDeclarator.h"

#include <algorithm>
#include <string>

#include "AbstractSyntaxTreeVisitor.h"
#include "ParameterList.h"

namespace ast {

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<Declarator> declaration) :
        FunctionDeclarator(std::move(declaration), std::unique_ptr<ParameterList> { new ParameterList() }) {
}

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<Declarator> declaration,
        std::unique_ptr<ParameterList> parameterList) :
        DirectDeclarator(declaration->getName(), declaration->getContext()),
        formalArguments { std::move(parameterList) } {
}

FunctionDeclarator::~FunctionDeclarator() {
}

void FunctionDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace ast */
