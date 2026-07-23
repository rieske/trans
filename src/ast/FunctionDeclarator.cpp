#include "FunctionDeclarator.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        baseDeclarator { std::move(declarator) }
{
}

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator, FormalArguments formalArguments,
        bool variadic) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        baseDeclarator { std::move(declarator) },
        formalArguments { std::move(formalArguments) },
        variadic { variadic }
{
}

bool FunctionDeclarator::isVariadic() const {
    return variadic;
}

void FunctionDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void FunctionDeclarator::visitFormalArguments(AbstractSyntaxTreeVisitor& visitor) {
    for (auto& argument : formalArguments) {
        argument.accept(visitor);
    }
}

void FunctionDeclarator::visitBaseDeclarator(AbstractSyntaxTreeVisitor& visitor) {
    baseDeclarator->accept(visitor);
}

const FormalArguments& FunctionDeclarator::getFormalArguments() const {
    return formalArguments;
}

DirectDeclarator& FunctionDeclarator::getBaseDeclarator() const {
    return *baseDeclarator;
}

type::Type FunctionDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) {
    // Outer pointers on a function declarator apply to the return type:
    //   int *f(void)  => function returning int*
    // Inner structure (e.g. NestedDeclarator with *) builds pointer-to-function:
    //   int (*f)(void) => pointer to function returning int
    type::Type returnType = baseType;
    for (Pointer pointer : indirection) {
        returnType = type::pointer(returnType, pointer.getQualifiers());
    }
    std::vector<type::Type> argumentTypes;
    for (const auto& argument : formalArguments) {
        argumentTypes.push_back(argument.getType());
    }
    type::Type functionType = type::function(returnType, argumentTypes, variadic);
    return baseDeclarator->getFundamentalType({}, functionType);
}

} // namespace ast
