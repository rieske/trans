#include "FunctionDefinition.h"

#include <algorithm>

#include "../code_generator/FunctionEntry.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "FunctionDeclarator.h"

namespace ast {

const std::string FunctionDefinition::ID { "<func_decl>" };

FunctionDefinition::FunctionDefinition(TypeSpecifier returnType, std::unique_ptr<FunctionDeclarator> declarator, std::unique_ptr<AbstractSyntaxTreeNode> body) :
        returnType { returnType },
        declarator { std::move(declarator) },
        body { std::move(body) }
{
}

FunctionDefinition::~FunctionDefinition() {
}

void FunctionDefinition::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void FunctionDefinition::visitDeclarator(AbstractSyntaxTreeVisitor& visitor) {
    declarator->accept(visitor);
}

void FunctionDefinition::setSymbol(code_generator::FunctionEntry symbol) {
    this->symbol = std::make_unique<code_generator::FunctionEntry>(symbol);
}

code_generator::FunctionEntry* FunctionDefinition::getSymbol() const {
    return symbol.get();
}

std::string FunctionDefinition::getName() const {
    return declarator->getName();
}

translation_unit::Context FunctionDefinition::getDeclarationContext() const {
    return declarator->getContext();
}

const std::vector<std::unique_ptr<FormalArgument> >& FunctionDefinition::getFormalArguments() const {
    return declarator->getFormalArguments();
}

}
