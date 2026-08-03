#ifndef BUILTINS_BSWAP_TABLE_H_
#define BUILTINS_BSWAP_TABLE_H_

// Product table for __builtin_bswap* (registry lookup + SA symbol install).

#include "types/Type.h"

namespace builtins {

struct BswapBuiltin {
    const char* name;
    int widthBytes;
};

constexpr BswapBuiltin kBswapBuiltins[] = {
        { "__builtin_bswap16", 2 },
        { "__builtin_bswap32", 4 },
        { "__builtin_bswap64", 8 },
};

inline type::Type bswapValueType(int widthBytes) {
    if (widthBytes == 2) {
        return type::unsignedShort();
    }
    if (widthBytes == 4) {
        return type::unsignedInteger();
    }
    return type::unsignedLong();
}

} // namespace builtins

#endif
