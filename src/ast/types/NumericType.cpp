#include "NumericType.h"

const int CHARACTER_ORDER { 1 };
const int INTEGER_ORDER { 4 };
const int FLOAT_ORDER { 8 };

namespace ast {

NumericType::NumericType(int sizeOrder) :
        BaseType { sizeOrder } {
}

std::unique_ptr<BaseType> NumericType::clone() const {
    return std::unique_ptr<BaseType> { new NumericType { sizeOrder } };
}

NumericType NumericType::createCharacter() {
    return CHARACTER_ORDER;
}

NumericType NumericType::createInteger() {
    return INTEGER_ORDER;
}

NumericType NumericType::createFloat() {
    return FLOAT_ORDER;
}

const NumericType& NumericType::convertTo(const BaseType& otherType) const {
    return otherType.convertFrom(*this);
}

const NumericType& NumericType::convertFrom(const NumericType& otherType) const {
    return otherType;
}

const Void& NumericType::convertFrom(const Void&) const {
    throw TypeConversionException { "can not convert void to numeric type" };
}

const Function& NumericType::convertFrom(const Function&) const {
    throw TypeConversionException { "can not convert function to numeric type" };
}

std::string NumericType::toString() const {
    switch (sizeOrder) {
    case CHARACTER_ORDER:
        return "char";
    case INTEGER_ORDER:
        return "int";
    case FLOAT_ORDER:
        return "float";
    default:
        throw std::runtime_error { "invalid type with size order: " + std::to_string(sizeOrder) };
    }
}

bool NumericType::isEqual(const BaseType& otherType) const noexcept {
    return otherType.isEqual(*this);
}

}
