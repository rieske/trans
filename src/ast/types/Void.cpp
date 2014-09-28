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

const Void& Void::promote(const BaseType& otherType) const {
    return otherType.promote(*this);
}

const NumericType& Void::promote(const NumericType&) const {
    throw TypePromotionException { "can't promote numeric type to void" };
}

const Void& Void::promote(const Void&) const {
    return *this;
}

} /* namespace ast */
