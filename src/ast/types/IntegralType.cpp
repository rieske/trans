#include "IntegralType.h"

namespace ast {

IntegralType::IntegralType(Integral type) :
        type { type }
{
}

std::unique_ptr<IntegralType> IntegralType::newSignedInteger() {
    return std::make_unique<IntegralType>(Integral::SIGNED_INT);
}

bool IntegralType::isNumeric() const {
    return true;
}

std::string IntegralType::toString() const {
    switch (type) {
    case Integral::SIGNED_CHAR:
        return "char";
    case Integral::UNSIGNED_CHAR:
        return "unsigned char";
    case Integral::SIGNED_SHORT:
        return "short";
    case Integral::UNSIGNED_SHORT:
        return "unsigned short";
    case Integral::SIGNED_INT:
        return "int";
    case Integral::UNSIGNED_INT:
        return "unsigned int";
    case Integral::SIGNED_LONG:
        return "long";
    case Integral::UNSIGNED_LONG:
        return "unsigned long";
    default:
        throw std::runtime_error { "unrecognized integral type in IntegralType::toString()" };
    }
}

IntegralType* IntegralType::clone() const {
    return new IntegralType { *this };
}

} /* namespace ast */
