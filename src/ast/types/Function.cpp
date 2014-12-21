#include "Function.h"

#include <memory>
#include <stdexcept>

#include "NumericType.h"

namespace ast {

Function::Function(std::vector<Type> argumentTypes) :
        BaseType { 0 },
        argumentTypes { argumentTypes } {
}

Function::~Function() {
}

std::unique_ptr<BaseType> Function::clone() const {
    return std::unique_ptr<BaseType> { new Function { argumentTypes } };
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
    return argumentTypes == anotherFunction.argumentTypes;
}

std::string Function::toString() const {
    throw std::runtime_error { "not implemented" };
}

bool Function::isEqual(const BaseType& otherType) const noexcept {
    return otherType.isEqual(*this);
}

bool Function::isEqual(const Function& otherFunction) const noexcept {
    return argumentTypes == otherFunction.argumentTypes;
}

} /* namespace ast */
