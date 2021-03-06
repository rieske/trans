#include "Function.h"
#include "Type.h"

namespace type {

Function::Function(std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Type>> arguments):
    returnType{std::move(returnType)},
    arguments{std::move(arguments)}
{
}

Function::Function(const Function& rhs):
    returnType{std::make_unique<Type>(*rhs.returnType)}
{
    for (const auto& arg: rhs.arguments) {
        arguments.push_back(std::make_unique<Type>(*arg));
    }
}

Type Function::getReturnType() const {
    return *returnType;
}


} // namespace type

