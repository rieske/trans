#include "Function.h"

#include <memory>
#include <stdexcept>

#include "NumericType.h"

namespace ast {

Function::Function(std::vector<Type> argumentTypes) :
        BaseType { 0 }, argumentTypes { argumentTypes } {
}

Function::~Function() {
}

std::unique_ptr<BaseType> Function::clone() const {
    return std::unique_ptr<BaseType> { new Function { argumentTypes } };
}

const BaseType& Function::convertTo(const BaseType& otherType) const {
    return otherType.convertFrom(*this);
}

const NumericType& Function::convertFrom(const NumericType&) const {
    throw TypeConversionException { "can not convert numeric type to function" };
}

const Void& Function::convertFrom(const Void&) const {
    throw TypeConversionException { "can not convert void type to function" };
}

const Function& Function::convertFrom(const Function& anotherFunction) const {
    if (argumentTypes != anotherFunction.argumentTypes) {
        throw TypeConversionException { "function arguments do not match" };
    }
    return *this;
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
