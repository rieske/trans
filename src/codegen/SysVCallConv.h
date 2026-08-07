#ifndef SYSVCALLCONV_H_
#define SYSVCALLCONV_H_

// System V AMD64 call-arg slots: independent GP (rdi..r9) and SSE (xmm0..7) budgets.

#include "Value.h"

#include <cstddef>

namespace codegen {

enum class SysVArgSlot { IntegerReg, SseReg, Stack };

struct SysVArgCounts {
    std::size_t integerRegs { 0 };
    std::size_t sseRegs { 0 };
};

struct SysVArgPlacement {
    SysVArgSlot slot;
    std::size_t index { 0 };
};

inline constexpr std::size_t SYSV_SSE_ARG_REGS = 8;

inline SysVArgPlacement takeSysVArgSlot(Type type, SysVArgCounts& used, std::size_t maxIntegerRegs) {
    if (type == Type::FLOATING) {
        if (used.sseRegs < SYSV_SSE_ARG_REGS) {
            const std::size_t index = used.sseRegs++;
            return { SysVArgSlot::SseReg, index };
        }
        return { SysVArgSlot::Stack };
    }
    if (used.integerRegs < maxIntegerRegs) {
        const std::size_t index = used.integerRegs++;
        return { SysVArgSlot::IntegerReg, index };
    }
    return { SysVArgSlot::Stack };
}

} // namespace codegen

#endif // SYSVCALLCONV_H_
