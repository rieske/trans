#include "FunctionDefinition.h"

#include <algorithm>

#include "../code_generator/FunctionEntry.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "FunctionDeclarator.h"
#include "ParameterList.h"

namespace ast {

const std::string FunctionDefinition::ID { "<func_decl>" };

FunctionDefinition::FunctionDefinition(TypeSpecifier returnType, std::unique_ptr<FunctionDeclarator> declaration,
        std::unique_ptr<AbstractSyntaxTreeNode> body) :
        returnType { returnType },
        declaration { std::move(declaration) },
        body { std::move(body) }
{
}

FunctionDefinition::~FunctionDefinition() {
}

void FunctionDefinition::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void FunctionDefinition::visitDeclaration(AbstractSyntaxTreeVisitor& visitor) {
    declaration->accept(visitor);
}

void FunctionDefinition::setSymbol(code_generator::FunctionEntry symbol) {
    this->symbol = std::make_unique<code_generator::FunctionEntry>(symbol);
}

code_generator::FunctionEntry* FunctionDefinition::getSymbol() const {
    return symbol.get();
}

std::string FunctionDefinition::getName() const {
    return declaration->getName();
}

translation_unit::Context FunctionDefinition::getDeclarationContext() const {
    return declaration->getContext();
}

const std::vector<std::unique_ptr<FormalArgument> >& FunctionDefinition::getDeclaredArguments() const {
    // FIXME:
    return declaration->formalArguments->getDeclaredParameters();
}

}
