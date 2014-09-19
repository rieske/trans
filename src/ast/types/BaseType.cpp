#include "BaseType.h"

#include <algorithm>

const int CHARACTER_ORDER { 1 };
const int INTEGER_ORDER { 4 };
const int FLOAT_ORDER { 8 };

namespace ast {

const BaseType BaseType::CHARACTER { CHARACTER_ORDER };
const BaseType BaseType::INTEGER { INTEGER_ORDER };
const BaseType BaseType::FLOAT { FLOAT_ORDER };

BaseType::BaseType(int sizeOrder) :
        sizeOrder { sizeOrder } {
}

bool BaseType::isInteger() const noexcept {
    return sizeOrder == INTEGER_ORDER;
}

BaseType BaseType::promote(BaseType type1, BaseType type2) {
    return std::max(type1.sizeOrder, type2.sizeOrder);
}

bool BaseType::operator==(const BaseType& rhs) const noexcept {
    return sizeOrder == rhs.sizeOrder;
}

bool BaseType::operator!=(const BaseType& rhs) const noexcept {
    return !(*this == rhs);
}

}
