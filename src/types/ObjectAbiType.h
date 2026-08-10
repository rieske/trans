#ifndef TYPES_OBJECTABITYPE_H_
#define TYPES_OBJECTABITYPE_H_

// Type-aware SysV object ABI helpers (requires Type.h).
// Size/word constants remain in ObjectAbi.h.

#include "ObjectAbi.h"
#include "SysVClassify.h"
#include "Type.h"

namespace type {
namespace object_abi {

// Complete records whose SysV class is MEMORY. Scalars and arrays never sret.
inline bool typeNeedsMemoryReturn(const type::Type& t) {
    return t.isCompleteRecord() && type::sysv::classify(t).memory;
}

} // namespace object_abi
} // namespace type

#endif // TYPES_OBJECTABITYPE_H_
