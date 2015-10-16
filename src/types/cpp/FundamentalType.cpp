#include "types/FundamentalType.h"

#include <stdexcept>

namespace ast {

bool FundamentalType::canConvertTo(const FundamentalType& otherType) const {
    //throw std::runtime_error { "ast::StoredType::canConvertTo(StoredType& otherType) not implemented" };
    return true;
}

bool FundamentalType::isVoid() const {
    return false;
}

bool FundamentalType::isNumeric() const {
    return false;
}

bool FundamentalType::isPointer() const {
    return false;
}

std::unique_ptr<FundamentalType> FundamentalType::dereference() const {
    throw std::runtime_error { "can not dereference non pointer type" };
}

}
