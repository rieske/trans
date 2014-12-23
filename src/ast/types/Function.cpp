#include "Function.h"

#include <memory>
#include <sstream>
#include <string>

#include "NumericType.h"

namespace ast {

Function::Function(Type returnType, std::vector<Type> argumentTypes) :
        BaseType { 0 },
        returnType { returnType },
        argumentTypes { argumentTypes }
{
}

Function::~Function() {
}

std::unique_ptr<BaseType> Function::clone() const {
    return std::unique_ptr<BaseType> { new Function { returnType, argumentTypes } };
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

std::string Function::toString() const {
    std::ostringstream oss;
    oss << returnType.toString() << " ( ";
    for (const auto& argumentType : argumentTypes) {
        oss << argumentType.toString() << " ";
    }
    oss << ")";
    return oss.str();
}

bool Function::isEqual(const BaseType& otherType) const noexcept {
    return otherType.isEqual(*this);
}

bool Function::isEqual(const Function& otherFunction) const noexcept {
    return returnType == otherFunction.returnType && argumentTypes == otherFunction.argumentTypes;
}

} /* namespace ast */
