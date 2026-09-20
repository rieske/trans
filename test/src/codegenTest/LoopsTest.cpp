#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Cfg.h"
#include "codegen/IrBuilders.h"
#include "codegen/Loops.h"

#include <string_view>

namespace {

using namespace testing;
using namespace codegen;

struct IrN {
    IrStringTable& t;
    int operator()(std::string_view s) const { return t.intern(s); }
};

const NaturalLoop* loopWithHeader(const std::vector<NaturalLoop>& loops, std::size_t header) {
    for (const auto& loop : loops) {
        if (loop.header == header) {
            return &loop;
        }
    }
    return nullptr;
}

TEST(Loops, predecessorsInvertSuccessors) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::zeroCompare(n("x")),
            ir::jump(n("L"), JumpCondition::IF_EQUAL),
            ir::inc(n("x")),
            ir::label(n("L")),
            ir::voidReturn(),
    });
    const auto pred = cfgPredecessors(cfg);
    ASSERT_THAT(pred, SizeIs(cfg.size()));
    EXPECT_THAT(pred[0], IsEmpty());
    for (std::size_t i = 0; i < cfg.size(); ++i) {
        for (const std::size_t s : cfgSuccessors(cfg, i)) {
            EXPECT_THAT(pred[s], Contains(i));
        }
    }
}

TEST(Loops, entryDominatesReachableBlocks) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::zeroCompare(n("x")),
            ir::jump(n("L"), JumpCondition::IF_EQUAL),
            ir::inc(n("x")),
            ir::label(n("L")),
            ir::voidReturn(),
    });
    const auto dom = dominators(cfg);
    ASSERT_THAT(dom, SizeIs(cfg.size()));
    for (std::size_t i = 0; i < cfg.size(); ++i) {
        EXPECT_THAT(dom[i], Contains(0u));
        EXPECT_THAT(dom[i], Contains(i));
    }
}

TEST(Loops, straightLineHasNoLoop) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::inc(n("x")),
            ir::ret(n("x")),
    });
    EXPECT_THAT(naturalLoops(cfg), IsEmpty());
}

TEST(Loops, ifDiamondHasNoLoop) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::zeroCompare(n("x")),
            ir::jump(n("Else"), JumpCondition::IF_EQUAL),
            ir::inc(n("a")),
            ir::jump(n("End")),
            ir::label(n("Else")),
            ir::inc(n("b")),
            ir::label(n("End")),
            ir::voidReturn(),
    });
    EXPECT_THAT(naturalLoops(cfg), IsEmpty());
}

TEST(Loops, forLoopHasHeaderLatchAndBody) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::assignConstant(n("0"), n("i")),
            ir::label(n("L")),
            ir::zeroCompare(n("i")),
            ir::jump(n("End"), JumpCondition::IF_EQUAL),
            ir::inc(n("i")),
            ir::jump(n("L")),
            ir::label(n("End")),
            ir::ret(n("i")),
    });
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_THAT(loops[0].header, Eq(1u));
    EXPECT_THAT(loops[0].blocks, UnorderedElementsAre(1u, 2u));
    EXPECT_THAT(loops[0].latches, ElementsAre(2u));
    const auto dom = dominators(cfg);
    EXPECT_THAT(dom[2], Contains(1u));
}

TEST(Loops, nestedLoopsInnerHeaderInOuter) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::label(n("Lo")),
            ir::zeroCompare(n("x")),
            ir::jump(n("X"), JumpCondition::IF_EQUAL),
            ir::label(n("Li")),
            ir::zeroCompare(n("y")),
            ir::jump(n("Iend"), JumpCondition::IF_EQUAL),
            ir::inc(n("y")),
            ir::jump(n("Li")),
            ir::label(n("Iend")),
            ir::inc(n("x")),
            ir::jump(n("Lo")),
            ir::label(n("X")),
            ir::voidReturn(),
    });
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(2));
    const NaturalLoop* inner = nullptr;
    const NaturalLoop* outer = nullptr;
    if (loops[0].blocks.size() < loops[1].blocks.size()) {
        inner = &loops[0];
        outer = &loops[1];
    } else {
        inner = &loops[1];
        outer = &loops[0];
    }
    EXPECT_THAT(outer->blocks, Contains(inner->header));
    for (const std::size_t b : inner->blocks) {
        EXPECT_THAT(outer->blocks, Contains(b));
    }
}

TEST(Loops, twoLatchesSameHeaderAreOneLoop) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::label(n("H")),
            ir::zeroCompare(n("x")),
            ir::jump(n("A"), JumpCondition::IF_EQUAL),
            ir::inc(n("a")),
            ir::jump(n("H")),
            ir::label(n("A")),
            ir::inc(n("b")),
            ir::jump(n("H")),
    });
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_THAT(loops[0].header, Eq(0u));
    EXPECT_THAT(loops[0].latches, UnorderedElementsAre(1u, 2u));
}

TEST(Loops, selfLoopIsOneLoop) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::label(n("L")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    });
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_THAT(loops[0].header, Eq(0u));
    EXPECT_THAT(loops[0].blocks, UnorderedElementsAre(0u));
    EXPECT_THAT(loops[0].latches, ElementsAre(0u));
}

TEST(Loops, irreducibleCycleIsNotANaturalLoop) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::zeroCompare(n("x")),
            ir::jump(n("A"), JumpCondition::IF_EQUAL),
            ir::jump(n("B")),
            ir::label(n("A")),
            ir::inc(n("i")),
            ir::jump(n("B")),
            ir::label(n("B")),
            ir::inc(n("j")),
            ir::jump(n("A")),
    });
    EXPECT_THAT(naturalLoops(cfg), IsEmpty());
}

TEST(Loops, deadJumpAfterReturnIsNotALoop) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::voidReturn(),
            ir::label(n("Dead")),
            ir::jump(n("L2")),
            ir::label(n("L2")),
            ir::voidReturn(),
    });
    EXPECT_THAT(naturalLoops(cfg), IsEmpty());
    const auto dom = dominators(cfg);
    ASSERT_THAT(dom, SizeIs(cfg.size()));
    EXPECT_THAT(dom[1], IsEmpty());
    EXPECT_THAT(dom[2], IsEmpty());
}

TEST(Loops, deadGotoIntoHeaderIsNotAnExtraLatch) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::label(n("L")),
            ir::inc(n("i")),
            ir::jump(n("L")),
            ir::label(n("Dead")),
            ir::jump(n("L")),
    });
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_THAT(loops[0].header, Eq(0u));
    EXPECT_THAT(loops[0].latches, ElementsAre(0u));
    EXPECT_THAT(loops[0].blocks, UnorderedElementsAre(0u));
}

TEST(Loops, unreachableIrreducibleIsNotALoop) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::voidReturn(),
            ir::label(n("A")),
            ir::inc(n("i")),
            ir::jump(n("B")),
            ir::label(n("B")),
            ir::inc(n("j")),
            ir::jump(n("A")),
    });
    EXPECT_THAT(naturalLoops(cfg), IsEmpty());
}

TEST(Loops, loopWithHeaderFindsForHeader) {
    IrStringTable strings;
    IrN n { strings };
    const Cfg cfg = buildCfg({
            ir::assignConstant(n("0"), n("i")),
            ir::label(n("L")),
            ir::zeroCompare(n("i")),
            ir::jump(n("End"), JumpCondition::IF_EQUAL),
            ir::inc(n("i")),
            ir::jump(n("L")),
            ir::label(n("End")),
            ir::ret(n("i")),
    });
    EXPECT_THAT(loopWithHeader(naturalLoops(cfg), 1u), NotNull());
    EXPECT_THAT(loopWithHeader(naturalLoops(cfg), 0u), IsNull());
}

} // namespace
