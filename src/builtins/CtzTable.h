#ifndef BUILTINS_CTZ_TABLE_H_
#define BUILTINS_CTZ_TABLE_H_

#include "types/Type.h"

namespace builtins {

struct CtzBuiltin {
    const char* name;
    int widthBytes;
};

constexpr CtzBuiltin kCtzBuiltins[] = {
        { "__builtin_ctz", 4 },
        { "__builtin_ctzl", 8 },
        { "__builtin_ctzll", 8 },
};

inline type::Type ctzArgType(int widthBytes) {
    return widthBytes == 4 ? type::unsignedInteger() : type::unsignedLong();
}

} // namespace builtins

#endif
