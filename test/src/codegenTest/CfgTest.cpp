#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Cfg.h"
#include "codegen/Instruction.h"

#include <stdexcept>
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

void expectIdentity(const IrStringTable& strings, const std::vector<Instruction>& body) {
    validateProcedureBody(body);
    EXPECT_THAT(dumpBody(strings, flattenCfg(buildCfg(body))), StrEq(dumpBody(strings, body)));
}

TEST(Cfg, unlabeledEntryVoidReturnIsOneBlock) {
    const std::vector<Instruction> body { ir::voidReturn() };

    const Cfg cfg = buildCfg(body);

    ASSERT_THAT(cfg, SizeIs(1));
    EXPECT_THAT(cfg[0].label, Eq(kNoSymbol));
    ASSERT_THAT(cfg[0].insts, SizeIs(1));
    EXPECT_THAT(cfg[0].insts[0].op, Eq(Op::VoidReturn));
    expectIdentity(IrStringTable {}, body);
}

TEST(Cfg, condJumpUnlabeledFallthroughAndLabeledDest) {
    IntermediateRepresentation names;
    IrN n { names.strings };
    const std::vector<Instruction> body {
            ir::zeroCompare(n("x")),
            ir::jump(n("else"), JumpCondition::IF_EQUAL),
            ir::assignConstant(n("1"), n("r")),
            ir::jump(n("end")),
            ir::label(n("else")),
            ir::assignConstant(n("0"), n("r")),
            ir::label(n("end")),
            ir::voidReturn(),
    };

    const Cfg cfg = buildCfg(body);

    ASSERT_THAT(cfg, SizeIs(4));
    EXPECT_THAT(cfg[0].label, Eq(kNoSymbol));
    EXPECT_THAT(cfg[0].insts.back().op, Eq(Op::Jump));
    EXPECT_THAT(cfg[1].label, Eq(kNoSymbol));
    EXPECT_THAT(cfg[1].insts.back().op, Eq(Op::Jump));
    EXPECT_THAT(cfg[2].label, Eq(n("else")));
    EXPECT_THAT(cfg[3].label, Eq(n("end")));
    for (const auto& block : cfg) {
        for (const auto& inst : block.insts) {
            EXPECT_THAT(inst.op, Ne(Op::Label));
        }
    }
    expectIdentity(names.strings, body);
}

TEST(Cfg, uncondGotoToLabeledDest) {
    IntermediateRepresentation names;
    IrN n { names.strings };
    const std::vector<Instruction> body {
            ir::jump(n("L")),
            ir::label(n("L")),
            ir::inc(n("x")),
            ir::voidReturn(),
    };

    const Cfg cfg = buildCfg(body);

    ASSERT_THAT(cfg, SizeIs(2));
    EXPECT_THAT(cfg[0].label, Eq(kNoSymbol));
    ASSERT_THAT(cfg[0].insts, SizeIs(1));
    EXPECT_THAT(cfg[0].insts[0].op, Eq(Op::Jump));
    EXPECT_THAT(cfg[1].label, Eq(n("L")));
    expectIdentity(names.strings, body);
}

TEST(Cfg, consecutiveLabelsMakeEmptyBlock) {
    IntermediateRepresentation names;
    IrN n { names.strings };
    const std::vector<Instruction> body {
            ir::label(n("L1")),
            ir::label(n("L2")),
            ir::inc(n("x")),
            ir::voidReturn(),
    };

    const Cfg cfg = buildCfg(body);

    ASSERT_THAT(cfg, SizeIs(2));
    EXPECT_THAT(cfg[0].label, Eq(n("L1")));
    EXPECT_THAT(cfg[0].insts, IsEmpty());
    EXPECT_THAT(cfg[1].label, Eq(n("L2")));
    ASSERT_THAT(cfg[1].insts, SizeIs(2));
    expectIdentity(names.strings, body);
}

TEST(Cfg, flattenKeepsJumpToNext) {
    IntermediateRepresentation names;
    IrN n { names.strings };
    const std::vector<Instruction> body {
            ir::jump(n("L")),
            ir::label(n("L")),
            ir::inc(n("x")),
    };

    EXPECT_THAT(dumpBody(names.strings, flattenCfg(buildCfg(body))), HasSubstr("GOTO L"));
    expectIdentity(names.strings, body);
}

TEST(Cfg, controlFlowShapeRoundTrips) {
    IntermediateRepresentation names;
    IrN n { names.strings };
    expectIdentity(names.strings, {
            ir::zeroCompare(n("x")),
            ir::jump(n("else"), JumpCondition::IF_EQUAL),
            ir::assignConstant(n("1"), n("r")),
            ir::jump(n("end")),
            ir::label(n("else")),
            ir::assignConstant(n("0"), n("r")),
            ir::label(n("end")),
            ir::valueCompare(n("a"), n("b")),
            ir::jump(n("loop"), JumpCondition::IF_BELOW),
            ir::jump(n("loop"), JumpCondition::IF_ABOVE),
            ir::jump(n("loop"), JumpCondition::IF_BELOW_OR_EQUAL),
            ir::jump(n("loop"), JumpCondition::IF_ABOVE_OR_EQUAL),
            ir::jump(n("loop"), JumpCondition::IF_NOT_EQUAL),
            ir::label(n("loop")),
            ir::inc(n("i")),
            ir::dec(n("i")),
            ir::inc(n("p"), 4),
            ir::dec(n("p"), 8),
    });
}

TEST(Cfg, validateRejectsUnlabeledAfterUncondTerminator) {
    IntermediateRepresentation names;
    IrN n { names.strings };
    EXPECT_THROW(validateProcedureBody({ ir::voidReturn(), ir::inc(n("x")) }), std::logic_error);

    Cfg cfg = buildCfg({ ir::voidReturn() });
    BasicBlock extra;
    extra.insts.push_back(ir::inc(n("x")));
    cfg.push_back(std::move(extra));
    EXPECT_THROW(validateCfg(cfg), std::logic_error);
}

} // namespace
