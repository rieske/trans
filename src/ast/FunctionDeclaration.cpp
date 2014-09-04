#include "FunctionDeclaration.h"

#include <algorithm>
#include <string>

#include "AbstractSyntaxTreeVisitor.h"
#include "ParameterList.h"

namespace ast {

FunctionDeclaration::FunctionDeclaration(std::unique_ptr<Declaration> declaration) :
        FunctionDeclaration(std::move(declaration), std::unique_ptr<ParameterList> { new ParameterList() }) {
}

FunctionDeclaration::FunctionDeclaration(std::unique_ptr<Declaration> declaration,
        std::unique_ptr<ParameterList> parameterList) :
        Declaration(declaration->getName(), "f" + declaration->getType(), declaration->getContext()),
        parameterList { std::move(parameterList) } {
}

FunctionDeclaration::~FunctionDeclaration() {
}

void FunctionDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace ast */