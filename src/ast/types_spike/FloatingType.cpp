#include "FloatingType.h"

namespace ast {

FloatingType::FloatingType(Floating type) :
        type { type }
{
}

FloatingType* FloatingType::clone() const {
    return new FloatingType { *this };
}

} /* namespace ast */
