#ifndef TYPES_OBJECTABITYPE_H_
#define TYPES_OBJECTABITYPE_H_

// Type-aware SysV object ABI helpers (requires Type.h).
// Size/word constants remain in ObjectAbi.h.

#include "ObjectAbi.h"
#include "Type.h"

namespace type {
namespace object_abi {

// Single sret policy: complete records larger than two integer registers.
// Scalars and arrays never sret (C does not return arrays by value).
// Incomplete records have size 0 and are excluded via isCompleteRecord().
inline bool typeNeedsMemoryReturn(const type::Type& t) {
    return t.isCompleteRecord() && needsMemoryReturn(t.getSize());
}

// Call/definition agreement: product skips sret for variadic functions.
inline bool typeNeedsMemoryReturn(const type::Type& t, bool calleeIsVariadic) {
    return !calleeIsVariadic && typeNeedsMemoryReturn(t);
}

} // namespace object_abi
} // namespace type

#endif // TYPES_OBJECTABITYPE_H_
