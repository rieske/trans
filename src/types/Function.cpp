#include "Function.h"
#include "Type.h"

#include <sstream>

namespace type {

struct Function::Impl {
    Impl(Type returnType, std::vector<Type> arguments, bool variadic) :
            returnType { std::move(returnType) },
            arguments { std::move(arguments) },
            variadic { variadic } {
    }

    Type returnType;
    std::vector<Type> arguments;
    bool variadic { false };
};

Function::Function(Type returnType, std::vector<Type> arguments, bool variadic) :
        impl_ { std::make_shared<Impl>(std::move(returnType), std::move(arguments), variadic) } {
}

const Type& Function::getReturnType() const {
    return impl_->returnType;
}

const std::vector<Type>& Function::getArguments() const {
    return impl_->arguments;
}

std::size_t Function::argumentCount() const {
    return impl_->arguments.size();
}

bool Function::isVariadic() const {
    return impl_->variadic;
}

std::string Function::to_string() const {
    std::stringstream str;
    str << impl_->returnType.to_string();
    str << "(";
    for (auto it = impl_->arguments.begin(); it != impl_->arguments.end(); ++it) {
        str << it->to_string();
        if (it < impl_->arguments.end() - 1) {
            str << ", ";
        }
    }
    if (impl_->variadic) {
        if (!impl_->arguments.empty()) {
            str << ", ";
        }
        str << "...";
    }
    str << ")";
    return str.str();
}

} // namespace type
