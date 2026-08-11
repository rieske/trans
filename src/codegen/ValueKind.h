#ifndef CODEGEN_VALUEKIND_H_
#define CODEGEN_VALUEKIND_H_

#include "codegen/Value.h"
#include "types/TypeQuery.h"

namespace codegen {

inline Type valueKindFromCType(const type::Type& t) {
    if (type::isComplex(t)) {
        return Type::COMPLEX;
    }
    return type::isFloating(t) ? Type::FLOATING : Type::INTEGRAL;
}

} // namespace codegen

#endif
