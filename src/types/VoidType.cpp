#include "VoidType.h"

namespace ast {

VoidType* VoidType::clone() const {
    return new VoidType { *this };
}

std::string VoidType::toString() const {
    return "void";
}

bool VoidType::isVoid() const {
    return true;
}

int VoidType::getSizeInBytes() const {
    return 0;
}

} /* namespace ast */
