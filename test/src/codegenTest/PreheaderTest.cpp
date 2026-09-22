#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Cfg.h"
#include "codegen/IrBuilders.h"
#include "codegen/Loops.h"
#include "codegen/Preheader.h"

#include <string>
#include <string_view>
#include <vector>

namespace {

using namespace testing;
using namespace codegen;

struct IrN {
    IrStringTable& t;
    int operator()(std::string_view s) const { return t.intern(s); }
};

std::string dumpBody(IrStringTable strings, const std::vector<Instruction>& body) {
    IntermediateRepresentation ir;
    ir.strings = std::move(strings);
    Procedure p;
    p.name = ir.strings.intern("f");
    p.body = body;
    ir.procedures.push_back(std::move(p));
    return toString(ir);
}

void expectRoundTrip(const IrStringTable& strings, const Cfg& cfg) {
    validateCfg(cfg);
    const auto flat = flattenCfg(cfg);
    validateProcedureBody(flat);
    EXPECT_THAT(dumpBody(strings, flattenCfg(buildCfg(flat))), StrEq(dumpBody(strings, flat)));
}

TEST(Preheader, alreadyHasIsIdentity) {
    IrStringTable strings;
    IrN n { strings };
    Cfg cfg = buildCfg({
            ir::assignConstant(n("0"), n("i")),
            ir::label(n("L")),
            ir::zeroCompare(n("i")),
            ir::jump(n("End"), JumpCondition::IF_EQUAL),
            ir::inc(n("i")),
            ir::jump(n("L")),
            ir::label(n("End")),
            ir::ret(n("i")),
    });
    const auto before = flattenCfg(cfg);
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Eq(0));
    EXPECT_THAT(flattenCfg(cfg), SizeIs(before.size()));
    expectRoundTrip(strings, cfg);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_TRUE(hasPreheader(cfg, loops[0]));
}

TEST(Preheader, selfLoopAtEntryInsertsBlockZero) {
    IrStringTable strings;
    IrN n { strings };
    const int header = n("L");
    Cfg cfg = buildCfg({
            ir::label(header),
            ir::inc(n("i")),
            ir::jump(header),
    });
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Eq(1));
    ASSERT_THAT(cfg, SizeIs(2));
    EXPECT_THAT(strings.get(cfg[0].label).compare(0, 3, "__L"), Eq(0));
    EXPECT_THAT(cfg[1].label, Eq(header));
    ASSERT_FALSE(cfg[1].insts.empty());
    EXPECT_THAT(cfg[1].insts.back().op, Eq(Op::Jump));
    EXPECT_THAT(cfg[1].insts.back().arg0, Eq(header));
    expectRoundTrip(strings, cfg);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_TRUE(hasPreheader(cfg, loops[0]));
    EXPECT_THAT(preheaderIndex(cfg, loops[0]), Optional(Eq(0u)));
}

TEST(Preheader, deadGotoIsRetargeted) {
    IrStringTable strings;
    IrN n { strings };
    const int header = n("L");
    Cfg cfg = buildCfg({
            ir::label(header),
            ir::inc(n("i")),
            ir::jump(header),
            ir::label(n("Dead")),
            ir::jump(header),
    });
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Eq(1));
    const int pre = cfg[0].label;
    bool sawDeadToPre = false;
    for (const auto& block : cfg) {
        if (block.label != n("Dead")) {
            continue;
        }
        ASSERT_FALSE(block.insts.empty());
        EXPECT_THAT(block.insts.back().arg0, Eq(pre));
        sawDeadToPre = true;
    }
    EXPECT_TRUE(sawDeadToPre);
    expectRoundTrip(strings, cfg);
}

TEST(Preheader, forInitPlusDeadGotoDoesNotInsert) {
    IrStringTable strings;
    IrN n { strings };
    const int header = n("L");
    Cfg cfg = buildCfg({
            ir::assignConstant(n("0"), n("i")),
            ir::label(header),
            ir::zeroCompare(n("i")),
            ir::jump(n("End"), JumpCondition::IF_EQUAL),
            ir::inc(n("i")),
            ir::jump(header),
            ir::label(n("End")),
            ir::ret(n("i")),
            ir::label(n("Dead")),
            ir::jump(header),
    });
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Eq(0));
    bool deadStillHeader = false;
    for (const auto& block : cfg) {
        if (block.label != n("Dead")) {
            continue;
        }
        ASSERT_FALSE(block.insts.empty());
        EXPECT_THAT(block.insts.back().arg0, Eq(header));
        deadStillHeader = true;
    }
    EXPECT_TRUE(deadStillHeader);
    expectRoundTrip(strings, cfg);
}

TEST(Preheader, twoLatchesKeepTargetingHeader) {
    IrStringTable strings;
    IrN n { strings };
    const int header = n("H");
    Cfg cfg = buildCfg({
            ir::label(header),
            ir::zeroCompare(n("x")),
            ir::jump(n("A"), JumpCondition::IF_EQUAL),
            ir::inc(n("a")),
            ir::jump(header),
            ir::label(n("A")),
            ir::inc(n("b")),
            ir::jump(header),
    });
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Eq(1));
    int headerJumps = 0;
    for (const auto& block : cfg) {
        for (const auto& inst : block.insts) {
            if (inst.op == Op::Jump && inst.arg0 == header) {
                ++headerJumps;
            }
        }
    }
    EXPECT_THAT(headerJumps, Eq(2));
    expectRoundTrip(strings, cfg);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_THAT(loops[0].latches, SizeIs(2));
    EXPECT_TRUE(hasPreheader(cfg, loops[0]));
}

TEST(Preheader, criticalEdgeFallthroughLandsInPre) {
    IrStringTable strings;
    IrN n { strings };
    Cfg cfg = buildCfg({
            ir::label(n("Lo")),
            ir::zeroCompare(n("x")),
            ir::jump(n("X"), JumpCondition::IF_EQUAL),
            ir::label(n("Li")),
            ir::inc(n("y")),
            ir::jump(n("Li")),
            ir::label(n("X")),
            ir::voidReturn(),
    });
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Eq(1));
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_TRUE(hasPreheader(cfg, loops[0]));
    const std::size_t pre = *preheaderIndex(cfg, loops[0]);
    EXPECT_THAT(cfg[pre].insts, IsEmpty());
    expectRoundTrip(strings, cfg);
}

TEST(Preheader, twoNonLatchPredsLandOnPre) {
    IrStringTable strings;
    IrN n { strings };
    const int header = n("L");
    Cfg cfg = buildCfg({
            ir::zeroCompare(n("x")),
            ir::jump(n("A"), JumpCondition::IF_EQUAL),
            ir::jump(header),
            ir::label(n("A")),
            ir::jump(header),
            ir::label(header),
            ir::inc(n("i")),
            ir::jump(header),
    });
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Eq(1));
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_TRUE(hasPreheader(cfg, loops[0]));
    const int pre = cfg[*preheaderIndex(cfg, loops[0])].label;
    int toPre = 0;
    for (const auto& block : cfg) {
        for (const auto& inst : block.insts) {
            if (inst.op == Op::Jump && inst.arg0 == pre) {
                ++toPre;
            }
        }
    }
    EXPECT_THAT(toPre, Ge(2));
    expectRoundTrip(strings, cfg);
}

TEST(Preheader, nestedEachHeaderGetsPreheader) {
    IrStringTable strings;
    IrN n { strings };
    Cfg cfg = buildCfg({
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
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Ge(1));
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(2));
    EXPECT_TRUE(hasPreheader(cfg, loops[0]));
    EXPECT_TRUE(hasPreheader(cfg, loops[1]));
    expectRoundTrip(strings, cfg);
}

TEST(Preheader, irreducibleIsNoOp) {
    IrStringTable strings;
    IrN n { strings };
    Cfg cfg = buildCfg({
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
    const auto before = flattenCfg(cfg);
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Eq(0));
    EXPECT_THAT(flattenCfg(cfg), SizeIs(before.size()));
    expectRoundTrip(strings, cfg);
}

TEST(Preheader, noTerminatorLatchGetsGotoHeader) {
    IrStringTable strings;
    IrN n { strings };
    const int header = n("H");
    Cfg cfg = buildCfg({
            ir::zeroCompare(n("y")),
            ir::jump(header, JumpCondition::IF_EQUAL),
            ir::inc(n("a")),
            ir::jump(n("End")),
            ir::label(n("Latch")),
            ir::inc(n("i")),
            ir::label(header),
            ir::inc(n("x")),
            ir::jump(n("Latch")),
            ir::label(n("End")),
            ir::voidReturn(),
    });
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Eq(1));
    bool latchGotosHeader = false;
    for (const auto& block : cfg) {
        if (block.label != n("Latch")) {
            continue;
        }
        ASSERT_FALSE(block.insts.empty());
        EXPECT_THAT(block.insts.back().op, Eq(Op::Jump));
        EXPECT_THAT(block.insts.back().arg0, Eq(header));
        latchGotosHeader = true;
    }
    EXPECT_TRUE(latchGotosHeader);
    expectRoundTrip(strings, cfg);
}

TEST(Preheader, condFallthroughLatchKeepsDedicatedGotoOnHeader) {
    IrStringTable strings;
    IrN n { strings };
    const int header = n("H");
    Cfg cfg = buildCfg({
            ir::zeroCompare(n("y")),
            ir::jump(header, JumpCondition::IF_EQUAL),
            ir::inc(n("a")),
            ir::jump(n("End")),
            ir::label(n("Latch")),
            ir::inc(n("i")),
            ir::zeroCompare(n("i")),
            ir::jump(n("Exit"), JumpCondition::IF_EQUAL),
            ir::label(header),
            ir::inc(n("x")),
            ir::jump(n("Latch")),
            ir::label(n("Exit")),
            ir::voidReturn(),
            ir::label(n("End")),
            ir::voidReturn(),
    });
    const PreheaderStats stats = insertPreheaders(cfg, strings);
    EXPECT_THAT(stats.inserted, Eq(1));
    bool dedicatedToHeader = false;
    bool latchIsOnlyJe = false;
    for (const auto& block : cfg) {
        if (block.label == n("Latch")) {
            ASSERT_FALSE(block.insts.empty());
            EXPECT_THAT(block.insts.back().op, Eq(Op::Jump));
            EXPECT_THAT(block.insts.back().cond, Eq(JumpCondition::IF_EQUAL));
            latchIsOnlyJe = true;
        }
        if (block.insts.size() == 1 && block.insts.front().op == Op::Jump
                && block.insts.front().cond == JumpCondition::UNCONDITIONAL
                && block.insts.front().arg0 == header) {
            dedicatedToHeader = true;
        }
    }
    EXPECT_TRUE(latchIsOnlyJe);
    EXPECT_TRUE(dedicatedToHeader);
    expectRoundTrip(strings, cfg);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_TRUE(hasPreheader(cfg, loops[0]));
}

} // namespace
