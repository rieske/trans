#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Cfg.h"
#include "codegen/Instruction.h"
#include "codegen/IrBuilders.h"
#include "codegen/IrPasses.h"
#include "codegen/Licm.h"
#include "codegen/Loops.h"
#include "codegen/Value.h"
#include "symbols/AddressPlan.h"

#include <initializer_list>
#include <string_view>

namespace {

using namespace testing;
using namespace codegen;

struct IrN {
    IrStringTable& t;
    int operator()(std::string_view s) const { return t.intern(s); }
};

Procedure makeProc(IrStringTable& strings, std::vector<Instruction> body, ProcedureFrame frame) {
    Procedure p;
    p.name = strings.intern("f");
    p.frame = std::move(frame);
    p.body = std::move(body);
    internProcedureTemps(strings, p);
    return p;
}

codegen::Value integral(IrStringTable& strings, std::string_view name, int size = 4) {
    return codegen::Value { strings.intern(name), 0, codegen::Type::INTEGRAL, size };
}

ProcedureFrame exprTemps(IrStringTable& strings, std::initializer_list<std::string_view> names) {
    ProcedureFrame frame;
    for (std::string_view name : names) {
        codegen::Value v = integral(strings, name);
        v.markExpressionTemp();
        frame.locals.push_back(std::move(v));
    }
    return frame;
}

ProcedureFrame withArgs(ProcedureFrame frame, IrStringTable& strings,
        std::initializer_list<std::string_view> args) {
    for (std::string_view name : args) {
        frame.arguments.push_back(integral(strings, name));
    }
    return frame;
}

bool instInBlock(const BasicBlock& block, Op op, int result) {
    for (const auto& inst : block.insts) {
        if (inst.op == op && inst.result == result) {
            return true;
        }
    }
    return false;
}

const NaturalLoop* loopAtLabel(const Cfg& cfg, const std::vector<NaturalLoop>& loops, int label) {
    for (const auto& loop : loops) {
        if (loop.header < cfg.size() && cfg[loop.header].label == label) {
            return &loop;
        }
    }
    return nullptr;
}

TEST(Licm, isLicmPureOpAllowList) {
    EXPECT_TRUE(isLicmPureOp(Op::Add));
    EXPECT_TRUE(isLicmPureOp(Op::AssignConstant));
    EXPECT_TRUE(isLicmPureOp(Op::FieldAddress));
    EXPECT_FALSE(isLicmPureOp(Op::AddressOf));
    EXPECT_FALSE(isLicmPureOp(Op::Div));
    EXPECT_FALSE(isLicmPureOp(Op::Dereference));
    EXPECT_FALSE(isLicmPureOp(Op::Inc));
    EXPECT_FALSE(isLicmPureOp(Op::LvalueAssign));
}

TEST(Licm, runIrPassesAtO0DoesNotHoist) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("a"), n("b"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "b", "i" })));
    ir = runIrPasses(std::move(ir), 0);
    EXPECT_THAT(toString(ir), HasSubstr("L:\n\tt := a + b"));
}

TEST(Licm, hoistsInvariantAddOfArguments) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("a"), n("b"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "b", "i" }));
    const LicmStats stats = hoistLoopInvariants(p, ir.strings);
    EXPECT_THAT(stats.hoisted, Eq(1));
    const Cfg cfg = buildCfg(p.body);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    ASSERT_TRUE(hasPreheader(cfg, loops[0]));
    const std::size_t pre = *preheaderIndex(cfg, loops[0]);
    EXPECT_TRUE(instInBlock(cfg[pre], Op::Add, n("t")));
    EXPECT_FALSE(instInBlock(cfg[loops[0].header], Op::Add, n("t")));
    bool pinned = false;
    for (const auto& local : p.frame.locals) {
        if (local.id() == n("t")) {
            EXPECT_FALSE(local.isExpressionTemp());
            pinned = true;
        }
    }
    EXPECT_TRUE(pinned);
}

TEST(Licm, hoistsIntoExistingForInitPreheader) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::assignConstant(n("0"), n("i")),
            ir::label(n("L")),
            ir::add(n("a"), n("b"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "b", "i" }));
    const LicmStats stats = hoistLoopInvariants(p, ir.strings);
    EXPECT_THAT(stats.inserted, Eq(0));
    EXPECT_THAT(stats.hoisted, Eq(1));
    const Cfg cfg = buildCfg(p.body);
    ASSERT_FALSE(cfg.empty());
    EXPECT_TRUE(instInBlock(cfg[0], Op::Add, n("t")));
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_FALSE(instInBlock(cfg[loops[0].header], Op::Add, n("t")));
}

TEST(Licm, hoistsDependentConstantsInOrder) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::assignConstant(n("1"), n("t1")),
            ir::add(n("n"), n("t1"), n("t2")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t1", "t2" }), ir.strings, { "n", "i" }));
    hoistLoopInvariants(p, ir.strings);
    const Cfg cfg = buildCfg(p.body);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    const std::size_t pre = *preheaderIndex(cfg, loops[0]);
    ASSERT_THAT(cfg[pre].insts.size(), Ge(2u));
    EXPECT_THAT(cfg[pre].insts[0].result, Eq(n("t1")));
    EXPECT_THAT(cfg[pre].insts[1].result, Eq(n("t2")));
    bool t1Pinned = false;
    for (const auto& local : p.frame.locals) {
        if (local.id() == n("t1")) {
            EXPECT_FALSE(local.isExpressionTemp());
            t1Pinned = true;
        }
    }
    EXPECT_TRUE(t1Pinned);
}

TEST(Licm, doesNotHoistAddOfInductionVar) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("i"), n("a"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "i" }));
    const LicmStats stats = hoistLoopInvariants(p, ir.strings);
    EXPECT_THAT(stats.hoisted, Eq(0));
    EXPECT_THAT(stats.loopsVisited, Eq(1));
}

TEST(Licm, doesNotHoistDereference) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::dereference(n("p"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "p", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, doesNotHoistDiv) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::div(n("a"), n("b"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "b", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, doesNotHoistNamedDest) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame;
    frame.locals.push_back(integral(ir.strings, "s"));
    frame.arguments.push_back(integral(ir.strings, "n"));
    frame.arguments.push_back(integral(ir.strings, "i"));
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("n"), n("n"), n("s")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, std::move(frame));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, doesNotHoistTwoDefs) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("a"), n("b"), n("t")),
            ir::add(n("a"), n("a"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "b", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, doesNotHoistTwoDefsInDifferentBlocks) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("a"), n("b"), n("t")),
            ir::jump(n("M")),
            ir::label(n("M")),
            ir::add(n("a"), n("a"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "b", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, doesNotHoistUseAfterExit) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("a"), n("b"), n("t")),
            ir::zeroCompare(n("i")),
            ir::jump(n("End"), JumpCondition::IF_EQUAL),
            ir::inc(n("i")),
            ir::jump(n("L")),
            ir::label(n("End")),
            ir::ret(n("t")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "b", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, doesNotHoistAddressTakenTemp) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("a"), n("b"), n("t")),
            ir::addressOf(n("t"), n("p")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t", "p" }), ir.strings, { "a", "b", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, doesNotHoistGlobalOperand) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("g"), n("a"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, doesNotHoistAddressOf) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::addressOf(n("a"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, doesNotHoistLeaFieldAddress) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::fieldAddress(n("a"), 4, n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, hoistsPointerBaseFieldAddress) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::fieldAddress(n("p"), 4, n("t"), symbols::AddressBaseMode::PointerValue),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "p", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(1));
}

TEST(Licm, refusesNamedOperandWhenLoopStores) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("n"), n("n"), n("t")),
            ir::lvalueAssign(n("t"), n("p")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "n", "p", "i" }));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(0));
}

TEST(Licm, copyPartDoesNotBlockNamedOperand) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "n", "i" });
    frame.locals.push_back(integral(ir.strings, "s"));
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("n"), n("n"), n("t")),
            ir::copyPart(n("n"), n("s"), 0),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, std::move(frame));
    EXPECT_THAT(hoistLoopInvariants(p, ir.strings).hoisted, Eq(1));
}

TEST(Licm, selfLoopWithAddInsertsPreheader) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::add(n("a"), n("b"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "b", "i" }));
    const LicmStats stats = hoistLoopInvariants(p, ir.strings);
    EXPECT_THAT(stats.inserted, Eq(1));
    EXPECT_THAT(stats.hoisted, Eq(1));
}

TEST(Licm, selfLoopWithOnlyIncDoesNotInsert) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame;
    frame.arguments.push_back(integral(ir.strings, "i"));
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, std::move(frame));
    const LicmStats stats = hoistLoopInvariants(p, ir.strings);
    EXPECT_THAT(stats.inserted, Eq(0));
    EXPECT_THAT(stats.hoisted, Eq(0));
}

TEST(Licm, twoSequentialWhilesGetOwnPreheaders) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("W1")),
            ir::add(n("a"), n("b"), n("t1")),
            ir::zeroCompare(n("i")),
            ir::jump(n("W2"), JumpCondition::IF_EQUAL),
            ir::inc(n("i")),
            ir::jump(n("W1")),
            ir::label(n("W2")),
            ir::add(n("a"), n("b"), n("t2")),
            ir::inc(n("j")),
            ir::jump(n("W2")),
    }, withArgs(exprTemps(ir.strings, { "t1", "t2" }), ir.strings, { "a", "b", "i", "j" }));
    hoistLoopInvariants(p, ir.strings);
    const Cfg cfg = buildCfg(p.body);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(2));
    const auto* w1 = loopAtLabel(cfg, loops, n("W1"));
    const auto* w2 = loopAtLabel(cfg, loops, n("W2"));
    ASSERT_THAT(w1, NotNull());
    ASSERT_THAT(w2, NotNull());
    ASSERT_TRUE(hasPreheader(cfg, *w1));
    ASSERT_TRUE(hasPreheader(cfg, *w2));
    const std::size_t p1 = *preheaderIndex(cfg, *w1);
    const std::size_t p2 = *preheaderIndex(cfg, *w2);
    EXPECT_THAT(p1, Ne(p2));
    EXPECT_TRUE(instInBlock(cfg[p1], Op::Add, n("t1")));
    EXPECT_TRUE(instInBlock(cfg[p2], Op::Add, n("t2")));
    EXPECT_FALSE(instInBlock(cfg[p2], Op::Add, n("t1")));
    EXPECT_FALSE(instInBlock(cfg[p1], Op::Add, n("t2")));
}

TEST(Licm, nestedOuterInvariantInIendGoesToOuterPreheader) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("Lo")),
            ir::zeroCompare(n("x")),
            ir::jump(n("X"), JumpCondition::IF_EQUAL),
            ir::label(n("Li")),
            ir::zeroCompare(n("y")),
            ir::jump(n("Iend"), JumpCondition::IF_EQUAL),
            ir::inc(n("y")),
            ir::jump(n("Li")),
            ir::label(n("Iend")),
            ir::add(n("a"), n("b"), n("t")),
            ir::inc(n("x")),
            ir::jump(n("Lo")),
            ir::label(n("X")),
            ir::voidReturn(),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "b", "x", "y" }));
    hoistLoopInvariants(p, ir.strings);
    const Cfg cfg = buildCfg(p.body);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(2));
    const NaturalLoop* outer = loops[0].blocks.size() > loops[1].blocks.size() ? &loops[0] : &loops[1];
    const std::size_t pre = *preheaderIndex(cfg, *outer);
    EXPECT_TRUE(instInBlock(cfg[pre], Op::Add, n("t")));
}

TEST(Licm, innerOnlyInvariantStaysInInnerPreheader) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("Lo")),
            ir::zeroCompare(n("x")),
            ir::jump(n("X"), JumpCondition::IF_EQUAL),
            ir::label(n("Li")),
            ir::add(n("y"), n("a"), n("t")),
            ir::zeroCompare(n("z")),
            ir::jump(n("Iend"), JumpCondition::IF_EQUAL),
            ir::inc(n("z")),
            ir::jump(n("Li")),
            ir::label(n("Iend")),
            ir::inc(n("y")),
            ir::inc(n("x")),
            ir::jump(n("Lo")),
            ir::label(n("X")),
            ir::voidReturn(),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "x", "y", "z" }));
    hoistLoopInvariants(p, ir.strings);
    const Cfg cfg = buildCfg(p.body);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(2));
    const NaturalLoop* inner = loops[0].blocks.size() < loops[1].blocks.size() ? &loops[0] : &loops[1];
    const NaturalLoop* outer = inner == &loops[0] ? &loops[1] : &loops[0];
    ASSERT_TRUE(preheaderIndex(cfg, *inner).has_value());
    EXPECT_TRUE(instInBlock(cfg[*preheaderIndex(cfg, *inner)], Op::Add, n("t")));
    if (const auto outerPre = preheaderIndex(cfg, *outer)) {
        EXPECT_FALSE(instInBlock(cfg[*outerPre], Op::Add, n("t")));
    }
}

void expectSameBody(const std::vector<Instruction>& a, const std::vector<Instruction>& b) {
    ASSERT_THAT(a, SizeIs(b.size()));
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_THAT(a[i].op, Eq(b[i].op));
        EXPECT_THAT(a[i].arg0, Eq(b[i].arg0));
        EXPECT_THAT(a[i].arg1, Eq(b[i].arg1));
        EXPECT_THAT(a[i].result, Eq(b[i].result));
    }
}

TEST(Licm, noLoopIsIdentity) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::add(n("a"), n("b"), n("t")),
            ir::ret(n("t")),
    }, withArgs(exprTemps(ir.strings, { "t" }), ir.strings, { "a", "b" }));
    const auto before = p.body;
    const LicmStats stats = hoistLoopInvariants(p, ir.strings);
    EXPECT_THAT(stats.hoisted, Eq(0));
    EXPECT_THAT(stats.inserted, Eq(0));
    EXPECT_THAT(stats.loopsVisited, Eq(0));
    expectSameBody(p.body, before);
}

TEST(Licm, forwardJumpsLeaveBodyUnchanged) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::zeroCompare(n("x")),
            ir::jump(n("Else"), JumpCondition::IF_EQUAL),
            ir::inc(n("a")),
            ir::jump(n("End")),
            ir::label(n("Else")),
            ir::inc(n("b")),
            ir::label(n("End")),
            ir::voidReturn(),
    }, withArgs(ProcedureFrame {}, ir.strings, { "x", "a", "b" }));
    const auto before = p.body;
    const LicmStats stats = hoistLoopInvariants(p, ir.strings);
    EXPECT_THAT(stats.hoisted, Eq(0));
    EXPECT_THAT(stats.inserted, Eq(0));
    EXPECT_THAT(stats.loopsVisited, Eq(0));
    expectSameBody(p.body, before);
}

TEST(Licm, twoBlocksHoistInIncreasingIndex) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure p = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::assignConstant(n("1"), n("t1")),
            ir::zeroCompare(n("i")),
            ir::jump(n("B"), JumpCondition::IF_EQUAL),
            ir::jump(n("C")),
            ir::label(n("B")),
            ir::assignConstant(n("2"), n("t2")),
            ir::jump(n("C")),
            ir::label(n("C")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(exprTemps(ir.strings, { "t1", "t2" }), ir.strings, { "i" }));
    hoistLoopInvariants(p, ir.strings);
    const Cfg cfg = buildCfg(p.body);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    const auto& pre = cfg[*preheaderIndex(cfg, loops[0])];
    ASSERT_THAT(pre.insts.size(), Ge(2u));
    EXPECT_THAT(pre.insts[0].result, Eq(n("t1")));
    EXPECT_THAT(pre.insts[1].result, Eq(n("t2")));
}

} // namespace
