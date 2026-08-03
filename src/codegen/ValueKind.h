#ifndef CODEGEN_VALUEKIND_H_
#define CODEGEN_VALUEKIND_H_

#include "codegen/Value.h"
#include "types/TypeQuery.h"

namespace codegen {

inline ValueKind valueKindFromCType(const type::Type& t) {
    if (type::isComplex(t)) {
        return ValueKind::COMPLEX;
    }
    return type::isFloating(t) ? ValueKind::FLOATING : ValueKind::INTEGRAL;
}

} // namespace codegen

#endif
