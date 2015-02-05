#include "Function.h"

#include <sstream>

namespace ast {

Function::Function(Type returnType, std::vector<std::pair<std::string, ast::Type>> arguments) :
        BaseType { 0 },
        returnType { returnType },
        arguments { arguments }
{
}

Function::Function(const Function& rhs) :
        BaseType(sizeOrder),
        returnType { rhs.returnType },
        arguments { rhs.arguments }
{
}

Function::~Function() {
}

Function& Function::operator=(const Function& assignFrom) {
    if (&assignFrom != this) {
        this->returnType = assignFrom.returnType;
        this->arguments = assignFrom.arguments;
    }
    return *this;
}

std::unique_ptr<BaseType> Function::clone() const {
    return std::unique_ptr<BaseType> { new Function { returnType, arguments } };
}

bool Function::canConvertTo(const BaseType& otherType) const noexcept {
    return otherType.canConvertTo(*this);
}

bool Function::canConvertTo(const NumericType&) const noexcept {
    return false;
}

bool Function::canConvertTo(const Void&) const noexcept {
    return false;
}

bool Function::canConvertTo(const Function& anotherFunction) const noexcept {
    return isEqual(anotherFunction);
}

const std::vector<std::pair<std::string, ast::Type>>& Function::getArguments() const {
    return arguments;
}

const Type& Function::getReturnType() const {
    return returnType;
}

std::string Function::toString() const {
    std::ostringstream oss;
    oss << returnType.toString() << " ( ";
    for (const auto& argument : arguments) {
        oss << argument.first << " " << argument.second.toString() << " ";
    }
    oss << ")";
    return oss.str();
}

bool Function::isEqual(const BaseType& otherType) const noexcept {
    return otherType.isEqual(*this);
}

bool Function::isEqual(const Function& otherFunction) const noexcept {
    return returnType == otherFunction.returnType && arguments == otherFunction.arguments;
}

} /* namespace ast */
