#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <vector>

#include "codegen/SysVCallConv.h"

namespace {

using namespace codegen;
using testing::ElementsAre;

constexpr std::size_t kMaxGp = SYSV_INTEGER_ARG_REGS;

std::vector<SysVArgSlot> classifyKinds(std::initializer_list<ValueKind> kinds, std::size_t maxGp = kMaxGp) {
    SysVArgCounts used;
    std::vector<SysVArgSlot> slots;
    slots.reserve(kinds.size());
    for (ValueKind kind : kinds) {
        slots.push_back(takeSysVArgSlot(kind, used, maxGp).slot);
    }
    return slots;
}

TEST(SysVCallConv, mixedIntFloatIntDoesNotSpendGpOnFloat) {
    // Width (movd vs movq) is emit-time; float32 and double are both FLOATING.
    SysVArgCounts used;
    const SysVArgPlacement fmt = takeSysVArgSlot(ValueKind::INTEGRAL, used, kMaxGp);
    const SysVArgPlacement d = takeSysVArgSlot(ValueKind::FLOATING, used, kMaxGp);
    const SysVArgPlacement code = takeSysVArgSlot(ValueKind::INTEGRAL, used, kMaxGp);

    EXPECT_EQ(fmt.slot, SysVArgSlot::IntegerReg);
    EXPECT_EQ(fmt.index, 0u);
    EXPECT_EQ(d.slot, SysVArgSlot::SseReg);
    EXPECT_EQ(d.index, 0u);
    EXPECT_EQ(code.slot, SysVArgSlot::IntegerReg);
    EXPECT_EQ(code.index, 1u);
}

TEST(SysVCallConv, seventhIntStacksWhileFloatStaysInXmm) {
    EXPECT_THAT(classifyKinds({
            ValueKind::INTEGRAL, ValueKind::INTEGRAL, ValueKind::INTEGRAL,
            ValueKind::INTEGRAL, ValueKind::INTEGRAL, ValueKind::INTEGRAL,
            ValueKind::INTEGRAL, ValueKind::FLOATING,
    }), ElementsAre(
            SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg,
            SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg,
            SysVArgSlot::Stack, SysVArgSlot::SseReg));
}

TEST(SysVCallConv, ninthFloatStacksWhileIntStaysInGp) {
    EXPECT_THAT(classifyKinds({
            ValueKind::FLOATING, ValueKind::FLOATING, ValueKind::FLOATING, ValueKind::FLOATING,
            ValueKind::FLOATING, ValueKind::FLOATING, ValueKind::FLOATING, ValueKind::FLOATING,
            ValueKind::FLOATING, ValueKind::INTEGRAL,
    }), ElementsAre(
            SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg,
            SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg,
            SysVArgSlot::Stack, SysVArgSlot::IntegerReg));
}

TEST(SysVCallConv, zeroIntegerBudgetStacksIntsKeepsXmm) {
    EXPECT_THAT(classifyKinds({ ValueKind::INTEGRAL, ValueKind::FLOATING }, 0),
            ElementsAre(SysVArgSlot::Stack, SysVArgSlot::SseReg));
}

TEST(SysVCallConv, eightFloatsFillXmmBudget) {
    std::vector<ValueKind> eight(SYSV_SSE_ARG_REGS, ValueKind::FLOATING);
    SysVArgCounts used;
    std::vector<std::size_t> indices;
    for (ValueKind kind : eight) {
        const SysVArgPlacement place = takeSysVArgSlot(kind, used, kMaxGp);
        ASSERT_EQ(place.slot, SysVArgSlot::SseReg);
        indices.push_back(place.index);
    }
    EXPECT_THAT(indices, ElementsAre(0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u));
    EXPECT_EQ(takeSysVArgSlot(ValueKind::FLOATING, used, kMaxGp).slot, SysVArgSlot::Stack);
}

TEST(SysVCallConv, namedOffsetsFollowIndependentBudgets) {
    SysVArgCounts used;
    used.integerRegs = 1;
    used.sseRegs = 1;
    EXPECT_EQ(sysvNamedGpOffset(used), type::object_abi::MACHINE_WORD_SIZE);
    EXPECT_EQ(sysvNamedFpOffset(used), SYSV_GP_SAVE_SIZE + SYSV_XMM_SAVE_STRIDE);

    used.integerRegs = SYSV_INTEGER_ARG_REGS + 3;
    used.sseRegs = SYSV_SSE_ARG_REGS + 1;
    EXPECT_EQ(sysvNamedGpOffset(used), SYSV_GP_SAVE_SIZE);
    EXPECT_EQ(sysvNamedFpOffset(used), SYSV_GP_SAVE_SIZE
            + static_cast<int>(SYSV_SSE_ARG_REGS) * SYSV_XMM_SAVE_STRIDE);
}

TEST(SysVCallConv, multiWordDoesNotSpendGp) {
    Value big { "big", 0, ValueKind::INTEGRAL, 24 };
    Value n { "n", 1, ValueKind::INTEGRAL, 8 };
    SysVArgCounts used;
    EXPECT_EQ(takeSysVArgSlot(big, used, kMaxGp).slot, SysVArgSlot::Stack);
    const SysVArgPlacement next = takeSysVArgSlot(n, used, kMaxGp);
    EXPECT_EQ(next.slot, SysVArgSlot::IntegerReg);
    EXPECT_EQ(next.index, 0u);
}

} // namespace
