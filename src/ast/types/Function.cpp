#include "Function.h"

#include <memory>
#include <sstream>
#include <string>

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
    std::ostringstream oss;
    oss << "function( ";
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
    return argumentTypes == otherFunction.argumentTypes;
}

} /* namespace ast */
