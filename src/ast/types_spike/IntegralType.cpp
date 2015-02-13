#include "IntegralType.h"

namespace ast {

IntegralType::IntegralType(Integral type) :
        type { type }
{
}

IntegralType* IntegralType::clone() const {
    return new IntegralType { *this };
}

} /* namespace ast */
