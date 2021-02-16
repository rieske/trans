#include "FunctionDefinition.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string FunctionDefinition::ID { "<function_definition>" };

FunctionDefinition::FunctionDefinition(
        DeclarationSpecifiers returnType,
        std::unique_ptr<Declarator> declarator,
        std::unique_ptr<AbstractSyntaxTreeNode> body)
:
        returnType { returnType },
        declarator { std::move(declarator) },
        body { std::move(body) }
{
}

void FunctionDefinition::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void FunctionDefinition::visitReturnType(AbstractSyntaxTreeVisitor& visitor) {
    returnType.accept(visitor);
}

void FunctionDefinition::visitDeclarator(AbstractSyntaxTreeVisitor& visitor) {
    declarator->accept(visitor);
}

void FunctionDefinition::visitBody(AbstractSyntaxTreeVisitor& visitor) {
    body->accept(visitor);
}

void FunctionDefinition::setSymbol(semantic_analyzer::FunctionEntry symbol) {
    this->symbol = std::make_unique<semantic_analyzer::FunctionEntry>(symbol);
}

semantic_analyzer::FunctionEntry* FunctionDefinition::getSymbol() const {
    if (!symbol) {
        throw std::runtime_error { "FunctionDefinition::getSymbol() == nullptr" };
    }
    return symbol.get();
}

std::string FunctionDefinition::getName() const {
    return declarator->getName();
}


void FunctionDefinition::setLocalVariables(std::map<std::string, semantic_analyzer::ValueEntry> localVariables) {
    this->localVariables = localVariables;
}

void FunctionDefinition::setArguments(std::vector<semantic_analyzer::ValueEntry> arguments) {
    this->arguments = arguments;
}

std::map<std::string, semantic_analyzer::ValueEntry> FunctionDefinition::getLocalVariables() const {
    return localVariables;
}

std::vector<semantic_analyzer::ValueEntry> FunctionDefinition::getArguments() const {
    return arguments;
}

} // namespace ast

