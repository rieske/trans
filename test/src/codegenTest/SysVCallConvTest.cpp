#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <vector>

#include "codegen/SysVCallConv.h"
#include "types/SysVClass.h"

namespace {

using namespace codegen;
using type::sysv::sseScalar;
using type::sysv::integerScalar;
using testing::ElementsAre;

constexpr std::size_t kMaxGp = 6;

SysVArgSlot firstSlot(const type::sysv::Classification& cls, SysVArgCounts& used,
        std::size_t maxGp = kMaxGp) {
    const SysVArgAssignment asgn = assignSysVArg(cls, used, maxGp);
    return asgn.onStack ? SysVArgSlot::Stack : asgn.slots[0];
}

std::vector<SysVArgSlot> classifyKinds(std::initializer_list<type::sysv::Classification> classes,
        std::size_t maxGp = kMaxGp) {
    SysVArgCounts used;
    std::vector<SysVArgSlot> slots;
    slots.reserve(classes.size());
    for (const auto& cls : classes) {
        slots.push_back(firstSlot(cls, used, maxGp));
    }
    return slots;
}

TEST(SysVCallConv, mixedIntFloatIntDoesNotSpendGpOnFloat) {
    SysVArgCounts used;
    const SysVArgAssignment fmt = assignSysVArg(integerScalar(), used, kMaxGp);
    const SysVArgAssignment d = assignSysVArg(sseScalar(), used, kMaxGp);
    const SysVArgAssignment code = assignSysVArg(integerScalar(), used, kMaxGp);

    ASSERT_FALSE(fmt.onStack);
    EXPECT_EQ(fmt.slots[0], SysVArgSlot::IntegerReg);
    EXPECT_EQ(fmt.indices[0], 0u);
    ASSERT_FALSE(d.onStack);
    EXPECT_EQ(d.slots[0], SysVArgSlot::SseReg);
    EXPECT_EQ(d.indices[0], 0u);
    ASSERT_FALSE(code.onStack);
    EXPECT_EQ(code.slots[0], SysVArgSlot::IntegerReg);
    EXPECT_EQ(code.indices[0], 1u);
}

TEST(SysVCallConv, seventhIntStacksWhileFloatStaysInXmm) {
    EXPECT_THAT(classifyKinds({
            integerScalar(), integerScalar(), integerScalar(),
            integerScalar(), integerScalar(), integerScalar(),
            integerScalar(), sseScalar(),
    }), ElementsAre(
            SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg,
            SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg, SysVArgSlot::IntegerReg,
            SysVArgSlot::Stack, SysVArgSlot::SseReg));
}

TEST(SysVCallConv, ninthFloatStacksWhileIntStaysInGp) {
    EXPECT_THAT(classifyKinds({
            sseScalar(), sseScalar(), sseScalar(), sseScalar(),
            sseScalar(), sseScalar(), sseScalar(), sseScalar(),
            sseScalar(), integerScalar(),
    }), ElementsAre(
            SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg,
            SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg, SysVArgSlot::SseReg,
            SysVArgSlot::Stack, SysVArgSlot::IntegerReg));
}

TEST(SysVCallConv, zeroIntegerBudgetStacksIntsKeepsXmm) {
    EXPECT_THAT(classifyKinds({ integerScalar(), sseScalar() }, 0),
            ElementsAre(SysVArgSlot::Stack, SysVArgSlot::SseReg));
}

TEST(SysVCallConv, memoryClassDoesNotSpendGp) {
    SysVArgCounts used;
    EXPECT_TRUE(assignSysVArg(type::sysv::memoryClass(), used, kMaxGp).onStack);
    const SysVArgAssignment gp = assignSysVArg(integerScalar(), used, kMaxGp);
    ASSERT_FALSE(gp.onStack);
    EXPECT_EQ(gp.slots[0], SysVArgSlot::IntegerReg);
    EXPECT_EQ(gp.indices[0], 0u);
}

TEST(SysVCallConv, twoIntegerEightbytesUseTwoGpRegs) {
    type::sysv::Classification cls;
    cls.count = 2;
    cls.eightbytes[0] = type::sysv::Class::Integer;
    cls.eightbytes[1] = type::sysv::Class::Integer;
    SysVArgCounts used;
    const SysVArgAssignment asgn = assignSysVArg(cls, used, kMaxGp);
    ASSERT_FALSE(asgn.onStack);
    EXPECT_EQ(asgn.count, 2);
    EXPECT_EQ(asgn.slots[0], SysVArgSlot::IntegerReg);
    EXPECT_EQ(asgn.indices[0], 0u);
    EXPECT_EQ(asgn.slots[1], SysVArgSlot::IntegerReg);
    EXPECT_EQ(asgn.indices[1], 1u);
    EXPECT_EQ(used.integerRegs, 2u);
    EXPECT_EQ(used.sseRegs, 0u);
}

TEST(SysVCallConv, twoIntegerEightbytesStackWhenOneGpLeft) {
    type::sysv::Classification cls;
    cls.count = 2;
    cls.eightbytes[0] = type::sysv::Class::Integer;
    cls.eightbytes[1] = type::sysv::Class::Integer;
    SysVArgCounts used;
    used.integerRegs = 5;
    const SysVArgAssignment asgn = assignSysVArg(cls, used, kMaxGp);
    EXPECT_TRUE(asgn.onStack);
    EXPECT_EQ(used.integerRegs, 5u);
}

TEST(SysVCallConv, mixedIntSseUsesBothBudgets) {
    type::sysv::Classification cls;
    cls.count = 2;
    cls.eightbytes[0] = type::sysv::Class::Integer;
    cls.eightbytes[1] = type::sysv::Class::Sse;
    SysVArgCounts used;
    const SysVArgAssignment asgn = assignSysVArg(cls, used, kMaxGp);
    ASSERT_FALSE(asgn.onStack);
    EXPECT_EQ(asgn.slots[0], SysVArgSlot::IntegerReg);
    EXPECT_EQ(asgn.indices[0], 0u);
    EXPECT_EQ(asgn.slots[1], SysVArgSlot::SseReg);
    EXPECT_EQ(asgn.indices[1], 0u);
    EXPECT_EQ(used.integerRegs, 1u);
    EXPECT_EQ(used.sseRegs, 1u);
}

TEST(SysVCallConv, eightFloatsFillXmmBudget) {
    SysVArgCounts used;
    std::vector<std::size_t> indices;
    for (std::size_t i = 0; i < SYSV_SSE_ARG_REGS; ++i) {
        const SysVArgAssignment asgn = assignSysVArg(sseScalar(), used, kMaxGp);
        ASSERT_FALSE(asgn.onStack);
        ASSERT_EQ(asgn.slots[0], SysVArgSlot::SseReg);
        indices.push_back(asgn.indices[0]);
    }
    EXPECT_THAT(indices, ElementsAre(0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u));
    EXPECT_TRUE(assignSysVArg(sseScalar(), used, kMaxGp).onStack);
}

TEST(SysVCallConv, x87IsNotRegisterPassable) {
    type::sysv::Classification cls;
    cls.count = 2;
    cls.eightbytes[0] = type::sysv::Class::X87;
    cls.eightbytes[1] = type::sysv::Class::X87Up;
    SysVArgCounts used;
    EXPECT_FALSE(cls.inRegisters());
    EXPECT_TRUE(assignSysVArg(cls, used, kMaxGp).onStack);
    EXPECT_EQ(used.integerRegs, 0u);
    EXPECT_EQ(used.sseRegs, 0u);
}

} // namespace
