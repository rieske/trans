#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <vector>

#include "codegen/SysVCallConv.h"

namespace {

using namespace codegen;
using testing::ElementsAre;

constexpr std::size_t kMaxGp = 6;

std::vector<SysVArgSlot> classifyTypes(std::initializer_list<Type> types, std::size_t maxGp = kMaxGp) {
    SysVArgCounts used;
    std::vector<SysVArgSlot> slots;
    slots.reserve(types.size());
    for (Type type : types) {
        slots.push_back(takeSysVArgSlot(type, used, maxGp).slot);
    }
    return slots;
}

TEST(SysVCallConv, mixedIntFloatIntDoesNotSpendGpOnFloat) {
    // Width (movd vs movq) is emit-time; float32 and double are both Type::FLOATING.
    SysVArgCounts used;
    const SysVArgPlacement fmt = takeSysVArgSlot(Type::INTEGRAL, used, kMaxGp);
    const SysVArgPlacement d = takeSysVArgSlot(Type::FLOATING, used, kMaxGp);
    const SysVArgPlacement code = takeSysVArgSlot(Type::INTEGRAL, used, kMaxGp);

    EXPECT_EQ(fmt.slot, SysVArgSlot::IntegerReg);
    EXPECT_EQ(fmt.index, 0u);
    EXPECT_EQ(d.slot, SysVArgSlot::SseReg);
    EXPECT_EQ(d.index, 0u);
    EXPECT_EQ(code.slot, SysVArgSlot::IntegerReg);
    EXPECT_EQ(code.index, 1u);
}

TEST(SysVCallConv, seventhIntStacksWhileFloatStaysInXmm) {
    EXPECT_THAT(classifyTypes({
            Type::INTEGRAL, Type::INTEGRAL, Type::INTEGRAL,
            Type::INTEGRAL, Type::INTEGRAL, Type::INTEGRAL,
            Type::INTEGRAL, Type::FLOATING,
    }), ElementsAre(
            SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg,
            SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg,
            SysVArgSlot::Stack, SysVArgSlot::SseReg));
}

TEST(SysVCallConv, ninthFloatStacksWhileIntStaysInGp) {
    EXPECT_THAT(classifyTypes({
            Type::FLOATING, Type::FLOATING, Type::FLOATING, Type::FLOATING,
            Type::FLOATING, Type::FLOATING, Type::FLOATING, Type::FLOATING,
            Type::FLOATING, Type::INTEGRAL,
    }), ElementsAre(
            SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg,
            SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg,
            SysVArgSlot::Stack, SysVArgSlot::IntegerReg));
}

TEST(SysVCallConv, zeroIntegerBudgetStacksIntsKeepsXmm) {
    EXPECT_THAT(classifyTypes({ Type::INTEGRAL, Type::FLOATING }, 0),
            ElementsAre(SysVArgSlot::Stack, SysVArgSlot::SseReg));
}

TEST(SysVCallConv, multiWordDoesNotSpendGp) {
    Value big { "big", 0, Type::INTEGRAL, 24 };
    Value n { "n", 1, Type::INTEGRAL, 8 };
    SysVArgCounts used;
    EXPECT_EQ(takeSysVArgSlot(big, used, kMaxGp).slot, SysVArgSlot::Stack);
    const SysVArgPlacement gp = takeSysVArgSlot(n, used, kMaxGp);
    EXPECT_EQ(gp.slot, SysVArgSlot::IntegerReg);
    EXPECT_EQ(gp.index, 0u);
}

TEST(SysVCallConv, eightFloatsFillXmmBudget) {
    std::vector<Type> eight(SYSV_SSE_ARG_REGS, Type::FLOATING);
    SysVArgCounts used;
    std::vector<std::size_t> indices;
    for (Type type : eight) {
        const SysVArgPlacement place = takeSysVArgSlot(type, used, kMaxGp);
        ASSERT_EQ(place.slot, SysVArgSlot::SseReg);
        indices.push_back(place.index);
    }
    EXPECT_THAT(indices, ElementsAre(0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u));
    EXPECT_EQ(takeSysVArgSlot(Type::FLOATING, used, kMaxGp).slot, SysVArgSlot::Stack);
}

} // namespace
