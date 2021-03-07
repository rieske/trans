#include "FunctionDeclarator.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator) :
        DirectDeclarator(declarator->getName(), declarator->getContext())
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

type::Type FunctionDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& returnType) {
    std::vector<type::Type> argumentTypes;
    for (const auto& argument : formalArguments) {
        argumentTypes.push_back(argument.getType());
    }
    type::Type type = type::function(returnType, argumentTypes);
    for (Pointer pointer : indirection) {
        type = type::pointer(type, pointer.getQualifiers());
    }
    return type;
}

} // namespace ast

