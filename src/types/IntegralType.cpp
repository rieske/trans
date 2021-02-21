#include "IntegralType.h"

#include <stdexcept>

namespace ast {

IntegralType::IntegralType(Integral type) :
        type { type }
{
}

std::unique_ptr<IntegralType> IntegralType::newSignedInteger() {
    return std::make_unique<IntegralType>(Integral::SIGNED_INT);
}

std::unique_ptr<IntegralType> IntegralType::newSignedChar() {
    return std::make_unique<IntegralType>(Integral::SIGNED_CHAR);
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

int IntegralType::getSizeInBytes() const {
    switch (type) {
    case Integral::SIGNED_CHAR:
        case Integral::UNSIGNED_CHAR:
        return 1;
    case Integral::SIGNED_SHORT:
        case Integral::UNSIGNED_SHORT:
        return 2;
    case Integral::SIGNED_INT:
        case Integral::UNSIGNED_INT:
        default:
        return 4;
    case Integral::SIGNED_LONG:
        case Integral::UNSIGNED_LONG:
        return 8;
    }
}

} // namespace ast

