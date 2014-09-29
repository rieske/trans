#include "Void.h"

#include <memory>

#include "NumericType.h"

namespace ast {

Void::Void() :
        BaseType { 0 } {
}

std::unique_ptr<BaseType> Void::clone() const {
    return std::unique_ptr<BaseType> { new Void { } };
}

Void::~Void() {
}

const Void& Void::convertTo(const BaseType& otherType) const {
    return otherType.convertFrom(*this);
}

const NumericType& Void::convertFrom(const NumericType&) const {
    throw TypeConversionException { "can't convert numeric type to void" };
}

const Void& Void::convertFrom(const Void&) const {
    return *this;
}

const Function& Void::convertFrom(const Function&) const {
    throw TypeConversionException { "can not convert function to void type" };
}

std::string Void::toString() const {
    return "void";
}

bool Void::isEqual(const BaseType& otherType) const noexcept {
    return otherType.isEqual(*this);
}

} /* namespace ast */
