#include "Function.h"
#include "Type.h"

#include <sstream>

namespace type {

Function::Function(std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Type>> arguments, bool variadic):
    returnType{std::move(returnType)},
    arguments{std::move(arguments)},
    variadic{variadic}
{
}

Function::Function(const Function& rhs):
    returnType{std::make_unique<Type>(*rhs.returnType)},
    variadic{rhs.variadic}
{
    for (const auto& arg: rhs.arguments) {
        arguments.push_back(std::make_unique<Type>(*arg));
    }
}

Function& Function::operator=(const Function& rhs) {
	if (this != &rhs) {
		returnType.reset(new Type(*rhs.returnType));
        arguments.clear();
        for (const auto& arg: rhs.arguments) {
            arguments.push_back(std::make_unique<Type>(*arg));
        }
        variadic = rhs.variadic;
	}
	return *this;
}

bool Function::isVariadic() const {
    return variadic;
}

Type Function::getReturnType() const {
    return *returnType;
}

std::vector<Type> Function::getArguments() const {
    std::vector<Type> args;
    for (const auto& arg: arguments) {
        args.push_back(*arg);
    }
    return args;
}

std::string Function::to_string() const {
    std::stringstream str;
    str << returnType->to_string();
    str << "(";
    for (auto it = arguments.begin(); it != arguments.end(); ++it) {
        str << (*it)->to_string();
        if (it < arguments.end()-1) {
            str << ", ";
        }
    }
    str << ")";
    return str.str();
}

} // namespace type

