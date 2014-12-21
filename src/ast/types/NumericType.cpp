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

bool NumericType::canConvertTo(const BaseType& otherType) const noexcept {
    return otherType.canConvertTo(*this);
}

bool NumericType::canConvertTo(const NumericType&) const noexcept {
    return true;
}

bool NumericType::canConvertTo(const Void&) const noexcept {
    return false;
}

bool NumericType::canConvertTo(const Function&) const noexcept {
    return false;
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
