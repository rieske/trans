#include "FunctionDeclarator.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        nested { std::move(declarator) }
{
}

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator, FormalArguments formalArguments,
        bool variadic) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        nested { std::move(declarator) },
        formalArguments { std::move(formalArguments) },
        variadic { variadic }
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

std::vector<std::string> FunctionDeclarator::parameterNames() const {
    std::vector<std::string> names;
    names.reserve(formalArguments.size());
    for (const auto& argument : formalArguments) {
        names.push_back(argument.getName());
    }
    return names;
}

const FunctionDeclarator* FunctionDeclarator::innermostFunctionDeclarator() const {
    if (const FunctionDeclarator* inner = nested->innermostFunctionDeclarator()) {
        return inner;
    }
    return this;
}

bool FunctionDeclarator::isVariadic() const {
    return variadic;
}

void FunctionDeclarator::forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn) {
    if (nested) {
        nested->forEachArrayDeclarator(fn);
    }
}

type::Type FunctionDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& returnType) const {
    // Outer pointers apply to the return type: `int *f()` is a function returning int*.
    type::Type actualReturn = returnType;
    for (Pointer pointer : indirection) {
        actualReturn = type::pointer(actualReturn, pointer.getQualifiers());
    }
    std::vector<type::Type> argumentTypes;
    for (const auto& argument : formalArguments) {
        argumentTypes.push_back(argument.getType());
    }
    type::Type functionType = type::function(actualReturn, argumentTypes, variadic);
    // Nested declarator may wrap further (e.g. `int (*f)()` → pointer to function).
    return nested->getFundamentalType({}, functionType);
}

} // namespace ast

