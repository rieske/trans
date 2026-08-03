#ifndef SYSVCALLCONV_H_
#define SYSVCALLCONV_H_

// System V AMD64 call-arg slots: independent GP (rdi..r9) and SSE (xmm0..7) budgets.

#include "Value.h"
#include "types/ObjectAbi.h"

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

inline constexpr std::size_t SYSV_INTEGER_ARG_REGS = 6;
inline constexpr std::size_t SYSV_SSE_ARG_REGS = 8;
// reg_save_area: GP qwords then XMM slots at 16-byte stride (SysV va_list).
inline constexpr int SYSV_GP_SAVE_SIZE =
        static_cast<int>(SYSV_INTEGER_ARG_REGS) * type::object_abi::MACHINE_WORD_SIZE;
inline constexpr int SYSV_XMM_SAVE_STRIDE = 16;
inline constexpr int SYSV_GP_OFFSET_LIMIT = SYSV_GP_SAVE_SIZE - type::object_abi::MACHINE_WORD_SIZE;
inline constexpr int SYSV_FP_OFFSET_LIMIT =
        SYSV_GP_SAVE_SIZE + static_cast<int>(SYSV_SSE_ARG_REGS) * SYSV_XMM_SAVE_STRIDE
        - SYSV_XMM_SAVE_STRIDE;

inline int sysvNamedGpOffset(const SysVArgCounts& used) {
    const std::size_t n = used.integerRegs < SYSV_INTEGER_ARG_REGS ? used.integerRegs : SYSV_INTEGER_ARG_REGS;
    return static_cast<int>(n) * type::object_abi::MACHINE_WORD_SIZE;
}

inline int sysvNamedFpOffset(const SysVArgCounts& used) {
    const std::size_t n = used.sseRegs < SYSV_SSE_ARG_REGS ? used.sseRegs : SYSV_SSE_ARG_REGS;
    return SYSV_GP_SAVE_SIZE + static_cast<int>(n) * SYSV_XMM_SAVE_STRIDE;
}

inline SysVArgPlacement takeSysVArgSlot(ValueKind kind, SysVArgCounts& used, std::size_t maxIntegerRegs) {
    if (kind == ValueKind::FLOATING) {
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

// Multi-word MEMORY class does not spend a GP or XMM slot.
inline SysVArgPlacement takeSysVArgSlot(const Value& v, SysVArgCounts& used, std::size_t maxIntegerRegs) {
    if (type::object_abi::valueWords(v.getSizeInBytes()) != 1) {
        return { SysVArgSlot::Stack };
    }
    return takeSysVArgSlot(v.getValueKind(), used, maxIntegerRegs);
}

} // namespace codegen

#endif // SYSVCALLCONV_H_
