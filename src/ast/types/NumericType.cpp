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

const NumericType& NumericType::promote(const BaseType& otherType) const {
    return otherType.promote(*this);
}

const NumericType& NumericType::promote(const NumericType& otherType) const {
    return this->sizeOrder > otherType.sizeOrder ? *this : otherType;
}

const Void& NumericType::promote(const Void&) const {
    throw TypePromotionException { "can't promote void to numeric type" };
}

}
