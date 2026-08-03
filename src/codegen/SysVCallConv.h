#ifndef SYSVCALLCONV_H_
#define SYSVCALLCONV_H_

// System V AMD64: independent GP (rdi..r9) and SSE (xmm0..7) budgets.

#include "types/ObjectAbi.h"
#include "types/SysVClass.h"

#include <array>
#include <cstddef>
#include <vector>

namespace codegen {

enum class SysVArgSlot { IntegerReg, SseReg, Stack };

struct SysVArgCounts {
    std::size_t integerRegs { 0 };
    std::size_t sseRegs { 0 };
};

struct SysVArgAssignment {
    bool onStack { false };
    int count { 0 };
    std::array<SysVArgSlot, 2> slots {};
    std::array<std::size_t, 2> indices {};
};

inline constexpr std::size_t SYSV_INTEGER_ARG_REGS = 6;
inline constexpr std::size_t SYSV_SSE_ARG_REGS = 8;
inline constexpr int SYSV_GP_SAVE_SIZE =
        static_cast<int>(SYSV_INTEGER_ARG_REGS) * type::object_abi::MACHINE_WORD_SIZE;
inline constexpr int SYSV_XMM_SAVE_STRIDE = 16;

inline int sysvNamedGpOffset(const SysVArgCounts& used) {
    const std::size_t n = used.integerRegs < SYSV_INTEGER_ARG_REGS ? used.integerRegs : SYSV_INTEGER_ARG_REGS;
    return static_cast<int>(n) * type::object_abi::MACHINE_WORD_SIZE;
}

inline int sysvNamedFpOffset(const SysVArgCounts& used) {
    const std::size_t n = used.sseRegs < SYSV_SSE_ARG_REGS ? used.sseRegs : SYSV_SSE_ARG_REGS;
    return SYSV_GP_SAVE_SIZE + static_cast<int>(n) * SYSV_XMM_SAVE_STRIDE;
}

inline SysVArgAssignment assignSysVArg(const type::sysv::Classification& cls, SysVArgCounts& used,
        std::size_t maxIntegerRegs) {
    SysVArgAssignment asgn;
    if (!cls.inRegisters()) {
        asgn.onStack = true;
        return asgn;
    }
    const int needGp = type::sysv::integerEightbytes(cls);
    const int needSse = type::sysv::sseEightbytes(cls);
    if (used.integerRegs + static_cast<std::size_t>(needGp) > maxIntegerRegs
            || used.sseRegs + static_cast<std::size_t>(needSse) > SYSV_SSE_ARG_REGS) {
        asgn.onStack = true;
        return asgn;
    }
    asgn.count = cls.count;
    for (int i = 0; i < cls.count; ++i) {
        if (type::sysv::isInteger(cls.eightbytes[static_cast<std::size_t>(i)])) {
            asgn.slots[static_cast<std::size_t>(i)] = SysVArgSlot::IntegerReg;
            asgn.indices[static_cast<std::size_t>(i)] = used.integerRegs++;
        } else {
            asgn.slots[static_cast<std::size_t>(i)] = SysVArgSlot::SseReg;
            asgn.indices[static_cast<std::size_t>(i)] = used.sseRegs++;
        }
    }
    return asgn;
}

struct SysVStackArg {
    int sizeBytes { 0 };
    int alignBytes { 8 };
};

struct SysVStackLayout {
    struct Slot {
        int offsetBytes { 0 };
        int sizeBytes { 0 };
    };
    std::vector<Slot> slots;
    int usedBytes { 0 };
    int totalBytes { 0 };
};

// Left-to-right stack args. Offset 0 is RSP+8 after call (16-aligned).
// usedBytes is the unpadded end (va_start overflow). totalBytes is 16-padded
// (outgoing RSP adjustment).
inline SysVStackLayout layoutSysVStackArgs(const std::vector<SysVStackArg>& args) {
    SysVStackLayout layout;
    int off = 0;
    for (const auto& arg : args) {
        const int slotAlign = arg.alignBytes > type::object_abi::MACHINE_WORD_SIZE
                ? type::object_abi::STACK_ALIGNMENT
                : type::object_abi::MACHINE_WORD_SIZE;
        const int slotSize =
                type::object_abi::valueWords(arg.sizeBytes) * type::object_abi::MACHINE_WORD_SIZE;
        off = (off + slotAlign - 1) & ~(slotAlign - 1);
        layout.slots.push_back({ off, slotSize });
        off += slotSize;
    }
    layout.usedBytes = off;
    layout.totalBytes = (off + type::object_abi::STACK_ALIGNMENT - 1)
            & ~(type::object_abi::STACK_ALIGNMENT - 1);
    return layout;
}

} // namespace codegen

#endif // SYSVCALLCONV_H_
