#include "Function.h"
#include "Type.h"

#include <sstream>

namespace type {

struct Function::Impl {
    std::unique_ptr<Type> returnType;
    std::vector<std::unique_ptr<Type>> arguments;
    bool variadic { false };
};

Function::Function(std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Type>> arguments,
        bool variadic) :
    impl_ { std::make_shared<Impl>() }
{
    impl_->returnType = std::move(returnType);
    impl_->arguments = std::move(arguments);
    impl_->variadic = variadic;
}

Type Function::getReturnType() const {
    return *impl_->returnType;
}

std::vector<Type> Function::getArguments() const {
    std::vector<Type> args;
    for (const auto& arg : impl_->arguments) {
        args.push_back(*arg);
    }
    return args;
}

std::size_t Function::argumentCount() const {
    return impl_->arguments.size();
}

bool Function::isVariadic() const {
    return impl_->variadic;
}

std::string Function::to_string() const {
    std::stringstream str;
    str << impl_->returnType->to_string();
    str << "(";
    for (auto it = impl_->arguments.begin(); it != impl_->arguments.end(); ++it) {
        str << (*it)->to_string();
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
