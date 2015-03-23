#include "FloatingType.h"

namespace ast {

FloatingType::FloatingType(Floating type) :
        type { type }
{
}

bool FloatingType::isNumeric() const {
    return true;
}

std::string FloatingType::toString() const {
    switch (type) {
    case Floating::FLOAT:
        return "float";
    case Floating::DOUBLE:
        return "double";
    case Floating::LONG_DOUBLE:
        return "long double";
    default:
        throw std::runtime_error { "unrecognized floating type in FloatingType::toString()" };
    }
}

int FloatingType::getSizeInBytes() const {
    switch (type) {
    case Floating::FLOAT:
        default:
        return 4;
    case Floating::DOUBLE:
        return 8;
    case Floating::LONG_DOUBLE:
        return 16;
    }
}

FloatingType* FloatingType::clone() const {
    return new FloatingType { *this };
}

} /* namespace ast */
