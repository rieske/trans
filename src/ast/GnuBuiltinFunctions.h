#ifndef AST_GNUBUILTINFUNCTIONS_H_
#define AST_GNUBUILTINFUNCTIONS_H_

#include "types/Type.h"

#include <string_view>

namespace ast {

struct GnuBswapBuiltin {
    const char* name;
    int widthBytes;
};

constexpr GnuBswapBuiltin kGnuBswapBuiltins[] = {
        { "__builtin_bswap16", 2 },
        { "__builtin_bswap32", 4 },
        { "__builtin_bswap64", 8 },
};

inline type::Type gnuBswapValueType(int widthBytes) {
    if (widthBytes == 2) {
        return type::unsignedShort();
    }
    if (widthBytes == 4) {
        return type::unsignedInteger();
    }
    return type::unsignedLong();
}

inline const GnuBswapBuiltin* findGnuBswapBuiltin(std::string_view name) {
    for (const auto& builtin : kGnuBswapBuiltins) {
        if (name == builtin.name) {
            return &builtin;
        }
    }
    return nullptr;
}

struct GnuCtzBuiltin {
    const char* name;
    int widthBytes;
};

constexpr GnuCtzBuiltin kGnuCtzBuiltins[] = {
        { "__builtin_ctz", 4 },
        { "__builtin_ctzl", 8 },
        { "__builtin_ctzll", 8 },
};

inline type::Type gnuCtzArgType(int widthBytes) {
    return widthBytes == 4 ? type::unsignedInteger() : type::unsignedLong();
}

inline const GnuCtzBuiltin* findGnuCtzBuiltin(std::string_view name) {
    for (const auto& builtin : kGnuCtzBuiltins) {
        if (name == builtin.name) {
            return &builtin;
        }
    }
    return nullptr;
}

inline bool isGnuAllocaBuiltin(std::string_view name) {
    return name == "__builtin_alloca";
}

} // namespace ast

#endif
