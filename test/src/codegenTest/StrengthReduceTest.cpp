#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Cfg.h"
#include "codegen/IrBuilders.h"
#include "codegen/Loops.h"
#include "codegen/StrengthReduce.h"
#include "codegen/Value.h"

#include <initializer_list>
#include <string_view>
#include <unordered_set>

namespace {

using namespace testing;
using namespace codegen;

struct IrN {
    IrStringTable& t;
    int operator()(std::string_view s) const { return t.intern(s); }
};

codegen::Value integral(IrStringTable& strings, std::string_view name) {
    return codegen::Value { strings.intern(name), 0, codegen::Type::INTEGRAL, 4 };
}

ProcedureFrame temps(IrStringTable& strings, std::initializer_list<std::string_view> names,
        bool expressionTemp) {
    ProcedureFrame frame;
    for (std::string_view name : names) {
        codegen::Value value = integral(strings, name);
        if (expressionTemp) {
            value.markExpressionTemp();
        }
        frame.locals.push_back(std::move(value));
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

Procedure makeProc(IrStringTable& strings, std::vector<Instruction> body, ProcedureFrame frame) {
    Procedure procedure;
    procedure.name = strings.intern("f");
    procedure.frame = std::move(frame);
    procedure.body = std::move(body);
    return procedure;
}

int countOpIn(const Cfg& cfg, const std::unordered_set<std::size_t>& blocks, Op op) {
    int n = 0;
    for (const std::size_t b : blocks) {
        if (b >= cfg.size()) {
            continue;
        }
        for (const auto& inst : cfg[b].insts) {
            if (inst.op == op) {
                ++n;
            }
        }
    }
    return n;
}

const Instruction* instAfter(const Cfg& cfg, Op op, int arg0) {
    for (const auto& block : cfg) {
        for (std::size_t i = 0; i < block.insts.size(); ++i) {
            if (block.insts[i].op == op && block.insts[i].arg0 == arg0 && i + 1 < block.insts.size()) {
                return &block.insts[i + 1];
            }
        }
    }
    return nullptr;
}

TEST(StrengthReduce, incMulBecomesAddOfFactor) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::assignConstant(n("0"), n("i")),
            ir::label(n("L")),
            ir::mul(n("i"), n("k"), n("t")),
            ir::add(n("s"), n("t"), n("s")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i", "s" }, false), ir.strings, { "k" }));
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();

    const StrengthReduceStats stats = strengthReduce(procedure, ir.strings);
    EXPECT_THAT(stats.reduced, Eq(1));
    EXPECT_THAT(stats.inserted, Eq(0));
    const Cfg cfg = buildCfg(procedure.body);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_THAT(countOpIn(cfg, loops[0].blocks, Op::Mul), Eq(0));
    const Instruction* init = nullptr;
    for (const auto& block : cfg) {
        for (const auto& inst : block.insts) {
            if (inst.op == Op::Mul) {
                init = &inst;
            }
        }
    }
    ASSERT_THAT(init, NotNull());
    EXPECT_THAT(init->arg0, Eq(n("i")));
    EXPECT_THAT(init->arg1, Eq(n("k")));
    const Instruction* bump = instAfter(cfg, Op::Inc, n("i"));
    ASSERT_THAT(bump, NotNull());
    EXPECT_THAT(bump->op, Eq(Op::Add));
    EXPECT_THAT(bump->arg0, Eq(bump->result));
    EXPECT_THAT(bump->arg1, Eq(n("k")));
    EXPECT_THAT(bump->result, Eq(init->result));
    bool pinned = false;
    for (const auto& local : procedure.frame.locals) {
        if (local.id() == bump->result) {
            EXPECT_FALSE(local.isExpressionTemp());
            pinned = true;
        }
    }
    EXPECT_TRUE(pinned);
}

TEST(StrengthReduce, stepTwoHasTwoPreheaderMuls) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::assignConstant(n("0"), n("i")),
            ir::assignConstant(n("2"), n("c")),
            ir::label(n("L")),
            ir::mul(n("i"), n("k"), n("t")),
            ir::add(n("i"), n("c"), n("i")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i", "c" }, false), ir.strings, { "k" }));
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();

    EXPECT_THAT(strengthReduce(procedure, ir.strings).reduced, Eq(1));
    const Cfg cfg = buildCfg(procedure.body);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_THAT(countOpIn(cfg, loops[0].blocks, Op::Mul), Eq(0));
    const Instruction* stepMul = nullptr;
    const Instruction* initMul = nullptr;
    for (std::size_t b = 0; b < cfg.size(); ++b) {
        if (loops[0].blocks.count(b) != 0) {
            continue;
        }
        for (const auto& inst : cfg[b].insts) {
            if (inst.op != Op::Mul) {
                continue;
            }
            const bool step = (inst.arg0 == n("c") && inst.arg1 == n("k"))
                    || (inst.arg0 == n("k") && inst.arg1 == n("c"));
            const bool init = (inst.arg0 == n("i") && inst.arg1 == n("k"))
                    || (inst.arg0 == n("k") && inst.arg1 == n("i"));
            if (step) {
                stepMul = &inst;
            }
            if (init) {
                initMul = &inst;
            }
        }
    }
    ASSERT_THAT(stepMul, NotNull());
    ASSERT_THAT(initMul, NotNull());
    const Instruction* bump = instAfter(cfg, Op::Add, n("i"));
    ASSERT_THAT(bump, NotNull());
    EXPECT_THAT(bump->op, Eq(Op::Add));
    EXPECT_THAT(bump->arg1, Eq(stepMul->result));
    EXPECT_THAT(bump->result, Eq(initMul->result));
}

void expectUnchanged(Procedure& procedure, IrStringTable& strings) {
    const auto before = procedure.body;
    const StrengthReduceStats stats = strengthReduce(procedure, strings);
    EXPECT_THAT(stats.reduced, Eq(0));
    EXPECT_THAT(stats.inserted, Eq(0));
    ASSERT_THAT(procedure.body, SizeIs(before.size()));
    for (std::size_t i = 0; i < before.size(); ++i) {
        EXPECT_THAT(procedure.body[i].op, Eq(before[i].op));
        EXPECT_THAT(procedure.body[i].arg0, Eq(before[i].arg0));
        EXPECT_THAT(procedure.body[i].arg1, Eq(before[i].arg1));
        EXPECT_THAT(procedure.body[i].result, Eq(before[i].result));
    }
}

TEST(StrengthReduce, doesNotReduceProductOfIvWithItself) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::mul(n("i"), n("i"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i" }, false), ir.strings, {}));
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();
    expectUnchanged(procedure, ir.strings);
}

TEST(StrengthReduce, doesNotReduceTwoUpdates) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::inc(n("i")),
            ir::mul(n("i"), n("k"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i" }, false), ir.strings, { "k" }));
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();
    expectUnchanged(procedure, ir.strings);
}

TEST(StrengthReduce, conditionalUpdateIsNotSimple) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::zeroCompare(n("c")),
            ir::jump(n("Skip"), JumpCondition::IF_EQUAL),
            ir::inc(n("i")),
            ir::label(n("Skip")),
            ir::mul(n("i"), n("k"), n("t")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i" }, false), ir.strings, { "c", "k" }));
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();
    expectUnchanged(procedure, ir.strings);
}

TEST(StrengthReduce, addressTakenFactorWithStoreIsNotReduced) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::addressOf(n("k"), n("p")),
            ir::label(n("L")),
            ir::lvalueAssign(n("v"), n("p")),
            ir::mul(n("i"), n("k"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i", "k", "p" }, false), ir.strings, { "v" }));
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();
    expectUnchanged(procedure, ir.strings);
}

TEST(StrengthReduce, addressTakenIvWithStoreIsNotReduced) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::assignConstant(n("1"), n("f")),
            ir::addressOf(n("i"), n("p")),
            ir::label(n("L")),
            ir::lvalueAssign(n("f"), n("p")),
            ir::mul(n("i"), n("f"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i", "p" }, false), ir.strings, {}));
    procedure.frame.locals.push_back(integral(ir.strings, "f"));
    procedure.frame.locals.back().markExpressionTemp();
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();
    expectUnchanged(procedure, ir.strings);
}

TEST(StrengthReduce, storeDoesNotBlockTempFactor) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::assignConstant(n("3"), n("f")),
            ir::label(n("L")),
            ir::lvalueAssign(n("f"), n("p")),
            ir::mul(n("i"), n("f"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i" }, false), ir.strings, { "p" }));
    procedure.frame.locals.push_back(integral(ir.strings, "f"));
    procedure.frame.locals.back().markExpressionTemp();
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();
    EXPECT_THAT(strengthReduce(procedure, ir.strings).reduced, Eq(1));
}

TEST(StrengthReduce, zeroStepDoesNotEmitDeadProduct) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::assignConstant(n("0"), n("i")),
            ir::label(n("L")),
            ir::mul(n("i"), n("k"), n("t")),
            ir::inc(n("i"), 0),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i" }, false), ir.strings, { "k" }));
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();
    EXPECT_THAT(strengthReduce(procedure, ir.strings).reduced, Eq(1));
    const Cfg cfg = buildCfg(procedure.body);
    int muls = 0;
    for (const auto& block : cfg) {
        for (const auto& inst : block.insts) {
            if (inst.op == Op::Mul) {
                ++muls;
                EXPECT_THAT(inst.arg0, Eq(n("i")));
                EXPECT_THAT(inst.arg1, Eq(n("k")));
            }
        }
    }
    EXPECT_THAT(muls, Eq(1));
    const Instruction* next = instAfter(cfg, Op::Inc, n("i"));
    ASSERT_THAT(next, NotNull());
    EXPECT_THAT(next->op, Eq(Op::Jump));
}

codegen::Value pointerSized(IrStringTable& strings, std::string_view name) {
    return codegen::Value { strings.intern(name), 0, codegen::Type::INTEGRAL, 8 };
}

TEST(StrengthReduce, indexStepsByStrideBytes) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame;
    frame.locals.push_back(pointerSized(ir.strings, "i"));
    Procedure procedure = makeProc(ir.strings, {
            ir::assignConstant(n("0"), n("i")),
            ir::label(n("L")),
            ir::indexAddress(n("a"), n("i"), 4, n("t"), symbols::AddressBaseMode::PointerValue),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, std::move(frame));
    procedure.frame.arguments.push_back(pointerSized(ir.strings, "a"));
    codegen::Value address = pointerSized(ir.strings, "t");
    address.markExpressionTemp();
    procedure.frame.locals.push_back(std::move(address));

    EXPECT_THAT(strengthReduce(procedure, ir.strings).reduced, Eq(1));
    const Cfg cfg = buildCfg(procedure.body);
    const auto loops = naturalLoops(cfg);
    ASSERT_THAT(loops, SizeIs(1));
    EXPECT_THAT(countOpIn(cfg, loops[0].blocks, Op::IndexAddress), Eq(0));
    const Instruction* bump = instAfter(cfg, Op::Inc, n("i"));
    ASSERT_THAT(bump, NotNull());
    EXPECT_THAT(bump->op, Eq(Op::PointerAdd));
    EXPECT_THAT(bump->imm, Eq(4));
    EXPECT_THAT(bump->arg0, Eq(bump->result));
    bool init = false;
    for (const auto& inst : cfg[0].insts) {
        if (inst.op == Op::IndexAddress && inst.arg0 == n("a") && inst.arg1 == n("i")
                && inst.imm == 4 && inst.result == bump->result) {
            init = true;
        }
    }
    EXPECT_TRUE(init);
}

TEST(StrengthReduce, narrowIndexStaysInLoop) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::assignConstant(n("0"), n("i")),
            ir::label(n("L")),
            ir::indexAddress(n("a"), n("i"), 4, n("t"), symbols::AddressBaseMode::PointerValue),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i" }, false), ir.strings, {}));
    procedure.frame.arguments.push_back(pointerSized(ir.strings, "a"));
    codegen::Value address = pointerSized(ir.strings, "t");
    address.markExpressionTemp();
    procedure.frame.locals.push_back(std::move(address));
    expectUnchanged(procedure, ir.strings);
}

TEST(StrengthReduce, insertsPreheaderWhenHeaderIsEntry) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::label(n("L")),
            ir::mul(n("i"), n("k"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, {}, false), ir.strings, { "i", "k" }));
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();
    const StrengthReduceStats stats = strengthReduce(procedure, ir.strings);
    EXPECT_THAT(stats.reduced, Eq(1));
    EXPECT_THAT(stats.inserted, Eq(1));
}

TEST(StrengthReduce, initializesFromCurrentIv) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    Procedure procedure = makeProc(ir.strings, {
            ir::assignConstant(n("1"), n("i")),
            ir::label(n("L")),
            ir::mul(n("i"), n("k"), n("t")),
            ir::inc(n("i")),
            ir::jump(n("L")),
    }, withArgs(temps(ir.strings, { "i" }, false), ir.strings, { "k" }));
    procedure.frame.locals.push_back(integral(ir.strings, "t"));
    procedure.frame.locals.back().markExpressionTemp();
    EXPECT_THAT(strengthReduce(procedure, ir.strings).reduced, Eq(1));
    const Cfg cfg = buildCfg(procedure.body);
    bool sawAssign = false;
    bool sawMulAfter = false;
    for (const auto& inst : cfg[0].insts) {
        if (inst.op == Op::AssignConstant && inst.result == n("i")) {
            sawAssign = true;
        }
        if (sawAssign && inst.op == Op::Mul && inst.arg0 == n("i") && inst.arg1 == n("k")) {
            sawMulAfter = true;
        }
    }
    EXPECT_TRUE(sawAssign);
    EXPECT_TRUE(sawMulAfter);
}

} // namespace
