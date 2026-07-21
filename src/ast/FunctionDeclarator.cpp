#include "FunctionDeclarator.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        nested { std::move(declarator) }
{
}

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator, FormalArguments formalArguments) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        nested { std::move(declarator) },
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

void FunctionDeclarator::visitNestedDeclarator(AbstractSyntaxTreeVisitor& visitor) {
    if (nested) {
        nested->accept(visitor);
    }
}

const FormalArguments& ast::FunctionDeclarator::getFormalArguments() const {
    return formalArguments;
}

type::Type FunctionDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& returnType) {
    // Outer pointers apply to the return type: `int *f()` is a function returning int*.
    type::Type actualReturn = returnType;
    for (Pointer pointer : indirection) {
        actualReturn = type::pointer(actualReturn, pointer.getQualifiers());
    }
    std::vector<type::Type> argumentTypes;
    for (const auto& argument : formalArguments) {
        argumentTypes.push_back(argument.getType());
    }
    type::Type functionType = type::function(actualReturn, argumentTypes);
    // Nested declarator may wrap further (e.g. `int (*f)()` → pointer to function).
    return nested->getFundamentalType({}, functionType);
}

} // namespace ast

