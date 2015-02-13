#include "VoidType.h"

namespace ast {

VoidType* VoidType::clone() const {
    return new VoidType { *this };
}

} /* namespace ast */
