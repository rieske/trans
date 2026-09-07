#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Instruction.h"
#include "codegen/IrPasses.h"
#include "codegen/Value.h"

#include <initializer_list>
#include <iterator>
#include <string_view>

#include "codegen/IrStringTable.h"

namespace {

using namespace testing;
using namespace codegen;

struct IrN {
    IrStringTable& t;
    int operator()(std::string_view s) const { return t.intern(s); }
};

Procedure makeProc(IrStringTable& strings, std::string_view name, std::vector<Instruction> body,
        ProcedureFrame frame = {}) {
    Procedure p;
    p.name = strings.intern(name);
    p.frame = std::move(frame);
    p.body = std::move(body);
    internProcedureTemps(strings, p);
    return p;
}

codegen::Value integral(IrStringTable& strings, std::string_view name, int size = 4) {
    return codegen::Value { strings.intern(name), 0, codegen::Type::INTEGRAL, size };
}

codegen::Value floating(IrStringTable& strings, std::string_view name, int size = 8) {
    return codegen::Value { strings.intern(name), 0, codegen::Type::FLOATING, size };
}

ProcedureFrame ints(IrStringTable& strings, std::initializer_list<std::string_view> names, int size = 4) {
    ProcedureFrame frame;
    for (std::string_view name : names) {
        frame.locals.push_back(integral(strings, name, size));
    }
    return frame;
}

ProcedureFrame exprTemps(IrStringTable& strings, std::initializer_list<std::string_view> names,
        int size = 4) {
    ProcedureFrame frame = ints(strings, names, size);
    for (auto& local : frame.locals) {
        local.markExpressionTemp();
    }
    return frame;
}

TEST(IrPasses, sealProcedures_padsFallOffEnd) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", { ir::assignConstant(n("1"), n("t0")) }));

    ir = sealProcedures(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt0 := 1\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, sealProcedures_doesNotPadExplicitReturn) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f",
            { ir::assignConstant(n("0"), n("t0")), ir::ret(n("t0")) },
            ProcedureFrame { { codegen::Value { n("t0"), 0, codegen::Type::INTEGRAL, 8 } }, {} }));

    ir = sealProcedures(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt0 := 0\n"
            "\tRETURN t0\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, sealProcedures_padsAfterFallthroughToExitLabel) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::zeroCompare(n("x")),
            ir::jump(n("end"), JumpCondition::IF_EQUAL),
            ir::inc(n("y")),
            ir::label(n("end")),
    }));

    ir = sealProcedures(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tCMP x, 0\n"
            "\tJE end\n"
            "\tINC y\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, applyCfgPasses_removesRedundantGoto) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::jump(n("L")),
            ir::label(n("L")),
            ir::inc(n("x")),
    }));

    ir = applyCfgPasses(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "L:\n"
            "\tINC x\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, applyCfgPasses_keepsConditionalAndNonAdjacent) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::jump(n("L"), JumpCondition::IF_EQUAL),
            ir::jump(n("M")),
            ir::label(n("L")),
            ir::inc(n("x")),
            ir::label(n("M")),
    }));

    ir = applyCfgPasses(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tJE L\n"
            "\tGOTO M\n"
            "L:\n"
            "\tINC x\n"
            "M:\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, applyCfgPasses_atO0KeepsLabeledDeadBlock) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::jump(n("end")),
            ir::label(n("dead")),
            ir::inc(n("x")),
            ir::label(n("end")),
            ir::voidReturn(),
    }));

    ir = applyCfgPasses(std::move(ir), 0);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tGOTO end\n"
            "dead:\n"
            "\tINC x\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, applyCfgPasses_atO0DoesNotThreadJumps) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::zeroCompare(n("x")),
            ir::jump(n("A"), JumpCondition::IF_EQUAL),
            ir::inc(n("y")),
            ir::jump(n("end")),
            ir::label(n("A")),
            ir::jump(n("B")),
            ir::label(n("B")),
            ir::inc(n("z")),
            ir::label(n("end")),
            ir::voidReturn(),
    }));

    ir = applyCfgPasses(std::move(ir), 0);

    EXPECT_THAT(toString(ir), HasSubstr("JE A"));
    EXPECT_THAT(toString(ir), HasSubstr("A:"));
}

TEST(IrPasses, applyCfgPasses_atO1ThreadsAndDropsTrampoline) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::zeroCompare(n("x")),
            ir::jump(n("A"), JumpCondition::IF_EQUAL),
            ir::inc(n("y")),
            ir::jump(n("end")),
            ir::label(n("A")),
            ir::jump(n("B")),
            ir::label(n("B")),
            ir::inc(n("z")),
            ir::label(n("end")),
            ir::voidReturn(),
    }));

    ir = applyCfgPasses(std::move(ir), 1);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tCMP x, 0\n"
            "\tJE B\n"
            "\tINC y\n"
            "\tGOTO end\n"
            "B:\n"
            "\tINC z\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, applyCfgPasses_atO1DropsLabeledDeadBlock) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::jump(n("end")),
            ir::label(n("dead")),
            ir::inc(n("x")),
            ir::label(n("end")),
            ir::voidReturn(),
    }));

    ir = applyCfgPasses(std::move(ir), 1);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, runIrPasses_atO0StillRemovesJumpToNext) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::jump(n("L")),
            ir::label(n("L")),
            ir::inc(n("x")),
    }));

    ir = runIrPasses(std::move(ir), 0);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "L:\n"
            "\tINC x\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, runIrPasses_composesSealAndPeephole) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::jump(n("done")),
            ir::label(n("done")),
    }));

    ir = runIrPasses(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "done:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_addsTwoAssignConstants) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::add(n("t1"), n("t2"), n("t3")),
            ir::ret(n("t3")),
    }, ints(ir.strings, { "t1", "t2", "t3" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt1 := 1\n"
            "\tt2 := 2\n"
            "\tt3 := 3\n"
            "\tRETURN t3\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_chainsInBlock) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::add(n("t1"), n("t2"), n("t3")),
            ir::add(n("t3"), n("t2"), n("t4")),
            ir::ret(n("t4")),
    }, ints(ir.strings, { "t1", "t2", "t3", "t4" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt1 := 1\n"
            "\tt2 := 2\n"
            "\tt3 := 3\n"
            "\tt4 := 5\n"
            "\tRETURN t4\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_tracksAssignOfKnownConstant) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t0")),
            ir::assign(n("t0"), n("a")),
            ir::assignConstant(n("2"), n("t1")),
            ir::assign(n("t1"), n("b")),
            ir::add(n("a"), n("b"), n("t2")),
            ir::ret(n("t2")),
    }, ints(ir.strings, { "t0", "a", "t1", "b", "t2" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt0 := 1\n"
            "\ta := t0\n"
            "\tt1 := 2\n"
            "\tb := t1\n"
            "\tt2 := 3\n"
            "\tRETURN t2\n"
            "ENDPROC f\n"));
}

// Fails if foldConstants is changed to collect escapes over the whole body up front, the way
// copyPropagate and eliminateDeadTemps do: x would be escaped from instruction 0, fold lost.

TEST(IrPasses, foldConstants_foldsBeforeAddressIsTakenLater) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("5"), n("x")),
            ir::assignConstant(n("1"), n("t1")),
            ir::add(n("x"), n("t1"), n("t2")),
            ir::addressOf(n("x"), n("p")),
            ir::ret(n("t2")),
    }, ints(ir.strings, { "x", "t1", "t2", "p" }, 8)));

    foldConstants(ir.procedures.front(), ir.strings);

    // x is still 5 where t2 is computed; the later &x cannot reach backwards.
    EXPECT_THAT(toString(ir), HasSubstr("\tt2 := 6\n"));
}

TEST(IrPasses, foldConstants_skipsMixedWithNonConst) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::add(n("t1"), n("x"), n("t2")),
            ir::ret(n("t2")),
    }, ints(ir.strings, { "t1", "t2", "x" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt1 := 1\n"
            "\tt2 := t1 + x\n"
            "\tRETURN t2\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_unaryMinusAndNot) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::unaryMinus(n("t1"), n("t2")),
            ir::assignConstant(n("0"), n("t3")),
            ir::unaryNot(n("t3"), n("t4")),
            ir::voidReturn(),
    }, ints(ir.strings, { "t1", "t2", "t3", "t4" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt1 := 1\n"
            "\tt2 := 0xffffffff\n"
            "\tt3 := 0\n"
            "\tt4 := 0xffffffff\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_divByZeroUnchanged) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("0"), n("t2")),
            ir::div(n("t1"), n("t2"), n("t3")),
            ir::ret(n("t3")),
    }, ints(ir.strings, { "t1", "t2", "t3" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt1 := 1\n"
            "\tt2 := 0\n"
            "\tt3 := t1 / t2\n"
            "\tRETURN t3\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_andHexMask) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("0xf"), n("t1")),
            ir::assignConstant(n("0x3"), n("t2")),
            ir::andOp(n("t1"), n("t2"), n("t3")),
            ir::ret(n("t3")),
    }, ints(ir.strings, { "t1", "t2", "t3" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt1 := 0xf\n"
            "\tt2 := 0x3\n"
            "\tt3 := 3\n"
            "\tRETURN t3\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_clearsAtLabel) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::jump(n("L")),
            ir::label(n("L")),
            ir::assignConstant(n("2"), n("t2")),
            ir::add(n("t1"), n("t2"), n("t3")),
            ir::ret(n("t3")),
    }, ints(ir.strings, { "t1", "t2", "t3" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt1 := 1\n"
            "\tGOTO L\n"
            "L:\n"
            "\tt2 := 2\n"
            "\tt3 := t1 + t2\n"
            "\tRETURN t3\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_skipsFloat) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame;
    frame.locals.push_back(floating(ir.strings, "t1"));
    frame.locals.push_back(floating(ir.strings, "t2"));
    frame.locals.push_back(floating(ir.strings, "t3"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("0x3ff0000000000000"), n("t1")),
            ir::assignConstant(n("0x4000000000000000"), n("t2")),
            ir::add(n("t1"), n("t2"), n("t3")),
            ir::ret(n("t3")),
    }, std::move(frame)));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), HasSubstr("t3 := t1 + t2"));
}

TEST(IrPasses, foldConstants_skipsEscapedLocal) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t0")),
            ir::assign(n("t0"), n("a")),
            ir::addressOf(n("a"), n("p")),
            ir::assignConstant(n("2"), n("t1")),
            ir::lvalueAssign(n("t1"), n("p")),
            ir::assignConstant(n("1"), n("t2")),
            ir::add(n("a"), n("t2"), n("t3")),
            ir::ret(n("t3")),
    }, ints(ir.strings, { "t0", "a", "p", "t1", "t2", "t3" }, 8)));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), HasSubstr("t3 := a + t2"));
}

TEST(IrPasses, applyCfgPasses_doesNotFold) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::add(n("t1"), n("t2"), n("t3")),
            ir::ret(n("t3")),
    }, ints(ir.strings, { "t1", "t2", "t3" })));

    ir = applyCfgPasses(std::move(ir), 1);

    EXPECT_THAT(toString(ir), HasSubstr("t3 := t1 + t2"));
}

TEST(IrPasses, runIrPasses_atO0DoesNotFold) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::add(n("t1"), n("t2"), n("t3")),
            ir::ret(n("t3")),
    }, ints(ir.strings, { "t1", "t2", "t3" })));

    ir = runIrPasses(std::move(ir), 0);

    EXPECT_THAT(toString(ir), HasSubstr("t3 := t1 + t2"));
}

TEST(IrPasses, runIrPasses_atO1Folds) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::add(n("t1"), n("t2"), n("t3")),
            ir::ret(n("t3")),
    }, ints(ir.strings, { "t1", "t2", "t3" })));

    ir = runIrPasses(std::move(ir), 1);

    EXPECT_THAT(toString(ir), HasSubstr("t3 := 3"));
    EXPECT_THAT(toString(ir), Not(HasSubstr("t1 + t2")));
}

TEST(IrPasses, eliminateDeadTemps_dropsUnusedAssignConstants) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::ret(n("t2")),
    }, exprTemps(ir.strings, { "t1", "t2" })));

    eliminateDeadTemps(ir.procedures.front());

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt2 := 2\n"
            "\tRETURN t2\n"
            "ENDPROC f\n"));
    EXPECT_EQ(ir.procedures.front().frame.locals.size(), 1u);
}

TEST(IrPasses, eliminateDeadTemps_dropsDeadAssignAndUnary) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assign(n("t1"), n("t2")),
            ir::unaryMinus(n("t2"), n("t3")),
            ir::voidReturn(),
    }, exprTemps(ir.strings, { "t1", "t2", "t3" })));

    eliminateDeadTemps(ir.procedures.front());

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
    EXPECT_TRUE(ir.procedures.front().frame.locals.empty());
}

TEST(IrPasses, eliminateDeadTemps_keepsJoinAssignsWithLaterUse) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::zeroCompare(n("x")),
            ir::jump(n("els"), JumpCondition::IF_EQUAL),
            ir::assignConstant(n("0"), n("t")),
            ir::jump(n("done")),
            ir::label(n("els")),
            ir::assignConstant(n("1"), n("t")),
            ir::label(n("done")),
            ir::ret(n("t")),
    }, exprTemps(ir.strings, { "x", "t" })));

    eliminateDeadTemps(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("t := 0"));
    EXPECT_THAT(toString(ir), HasSubstr("t := 1"));
}

TEST(IrPasses, eliminateDeadTemps_keepsNamedLocal) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame;
    frame.locals.push_back(integral(ir.strings, "t0"));
    frame.locals.back().markExpressionTemp();
    frame.locals.push_back(integral(ir.strings, "a"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t0")),
            ir::assign(n("t0"), n("a")),
            ir::voidReturn(),
    }, std::move(frame)));

    eliminateDeadTemps(ir.procedures.front());

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt0 := 1\n"
            "\ta := t0\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, eliminateDeadTemps_keepsAddressTaken) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t0", "p", "t1" }, 8);
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t0")),
            ir::addressOf(n("t0"), n("p")),
            ir::assignConstant(n("2"), n("t0")),
            ir::dereference(n("p"), n("p"), n("t1")),
            ir::ret(n("t1")),
    }, std::move(frame)));

    eliminateDeadTemps(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("t0 := 2"));
    EXPECT_THAT(toString(ir), HasSubstr("t0 := 1"));
}

TEST(IrPasses, eliminateDeadTemps_dropsUnusedAdd) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::add(n("x"), n("t1"), n("t2")),
            ir::ret(n("x")),
    }, exprTemps(ir.strings, { "t1", "t2", "x" })));

    eliminateDeadTemps(ir.procedures.front());

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tRETURN x\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, eliminateDeadTemps_dropsUnusedPointerOffset) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("i")),
            ir::pointerOffset(n("p"), n("i"), 4, n("q"), false),
            ir::ret(n("p")),
    }, exprTemps(ir.strings, { "i", "p", "q" }, 8)));

    eliminateDeadTemps(ir.procedures.front());

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tRETURN p\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, eliminateDeadTemps_keepsInc) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::inc(n("x")),
            ir::voidReturn(),
    }, exprTemps(ir.strings, { "x" })));

    eliminateDeadTemps(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("INC x"));
}

TEST(IrPasses, eliminateDeadTemps_keepsStore) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t")),
            ir::lvalueAssign(n("t"), n("p")),
            ir::voidReturn(),
    }, exprTemps(ir.strings, { "t", "p" })));

    eliminateDeadTemps(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("t := 1"));
    EXPECT_THAT(toString(ir), HasSubstr("*p := t"));
}

TEST(IrPasses, eliminateDeadTemps_keepsCall) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::call(n("g")),
            ir::voidReturn(),
    }));

    eliminateDeadTemps(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("CALL g"));
}

TEST(IrPasses, applyCfgPasses_doesNotDropDeadTemps) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::ret(n("t2")),
    }, exprTemps(ir.strings, { "t1", "t2" })));

    ir = applyCfgPasses(std::move(ir), 1);

    EXPECT_THAT(toString(ir), HasSubstr("t1 := 1"));
}

TEST(IrPasses, runIrPasses_atO0DoesNotDropDeadTemps) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::ret(n("t2")),
    }, exprTemps(ir.strings, { "t1", "t2" })));

    ir = runIrPasses(std::move(ir), 0);

    EXPECT_THAT(toString(ir), HasSubstr("t1 := 1"));
}

TEST(IrPasses, runIrPasses_atO1FoldsThenDropsDeadTemps) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::add(n("t1"), n("t2"), n("t3")),
            ir::ret(n("t3")),
    }, exprTemps(ir.strings, { "t1", "t2", "t3" })));

    ir = runIrPasses(std::move(ir), 1);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt3 := 3\n"
            "\tRETURN t3\n"
            "ENDPROC f\n"));
    EXPECT_EQ(ir.procedures.front().frame.locals.size(), 1u);
}

TEST(IrPasses, copyPropagate_rewritesTempCopyIntoAdd) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t1", "t2", "t3" });
    frame.locals.push_back(integral(ir.strings, "x"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assign(n("t1"), n("t2")),
            ir::add(n("t2"), n("x"), n("t3")),
            ir::ret(n("t3")),
    }, std::move(frame)));

    copyPropagate(ir.procedures.front());

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt2 := t1\n"
            "\tt3 := t1 + x\n"
            "\tRETURN t3\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, copyPropagate_keepsNamedLocal) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t1", "t3" });
    frame.locals.push_back(integral(ir.strings, "a"));
    frame.locals.push_back(integral(ir.strings, "x"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assign(n("t1"), n("a")),
            ir::add(n("a"), n("x"), n("t3")),
            ir::ret(n("t3")),
    }, std::move(frame)));

    copyPropagate(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("t3 := a + x"));
}

TEST(IrPasses, copyPropagate_skipsSignednessChangingAssign) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    type::sysv::Classification signExt;
    signExt.gprExtend = type::sysv::GprExtend::Sign;
    type::sysv::Classification zeroExt;
    zeroExt.gprExtend = type::sysv::GprExtend::Zero;
    ProcedureFrame frame;
    frame.locals.push_back(codegen::Value { n("t1"), 0, codegen::Type::INTEGRAL, 1, signExt });
    frame.locals.back().markExpressionTemp();
    frame.locals.push_back(codegen::Value { n("t2"), 0, codegen::Type::INTEGRAL, 1, zeroExt });
    frame.locals.back().markExpressionTemp();
    frame.locals.push_back(integral(ir.strings, "t3"));
    frame.locals.back().markExpressionTemp();
    frame.locals.push_back(integral(ir.strings, "x"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assign(n("t1"), n("t2")),
            ir::add(n("t2"), n("x"), n("t3")),
            ir::ret(n("t3")),
    }, std::move(frame)));

    copyPropagate(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("t3 := t2 + x"));
}

TEST(IrPasses, copyPropagate_skipsAddressTakenSrc) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t1", "t2", "t3", "p" }, 8);
    frame.locals.push_back(integral(ir.strings, "x", 8));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::addressOf(n("t1"), n("p")),
            ir::assign(n("t1"), n("t2")),
            ir::add(n("t2"), n("x"), n("t3")),
            ir::ret(n("t3")),
    }, std::move(frame)));

    copyPropagate(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("t3 := t2 + x"));
}

TEST(IrPasses, copyPropagate_doesNotRewriteAddressOf) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assign(n("t1"), n("t2")),
            ir::addressOf(n("t2"), n("p")),
            ir::ret(n("p")),
    }, exprTemps(ir.strings, { "t1", "t2", "p" }, 8)));

    copyPropagate(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("p := &t2"));
}

TEST(IrPasses, copyPropagate_clearsAtCall) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t1", "t2", "t3", "t4" });
    frame.locals.push_back(integral(ir.strings, "x"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assign(n("t1"), n("t2")),
            ir::argument(n("t2")),
            ir::call(n("g")),
            ir::retrieve(n("t3")),
            ir::add(n("t2"), n("x"), n("t4")),
            ir::ret(n("t4")),
    }, std::move(frame)));

    copyPropagate(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("PARAM t1"));
    EXPECT_THAT(toString(ir), HasSubstr("t4 := t2 + x"));
}

TEST(IrPasses, copyPropagate_clearsAtLabel) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t1", "t2", "t3" });
    frame.locals.push_back(integral(ir.strings, "x"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assign(n("t1"), n("t2")),
            ir::label(n("L")),
            ir::add(n("t2"), n("x"), n("t3")),
            ir::ret(n("t3")),
    }, std::move(frame)));

    copyPropagate(ir.procedures.front());

    EXPECT_THAT(toString(ir), HasSubstr("t3 := t2 + x"));
}

TEST(IrPasses, applyCfgPasses_doesNotCopyPropagate) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t1", "t2", "t3" });
    frame.locals.push_back(integral(ir.strings, "x"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assign(n("t1"), n("t2")),
            ir::add(n("t2"), n("x"), n("t3")),
            ir::ret(n("t3")),
    }, std::move(frame)));

    ir = applyCfgPasses(std::move(ir), 1);

    EXPECT_THAT(toString(ir), HasSubstr("t3 := t2 + x"));
}

TEST(IrPasses, runIrPasses_atO0DoesNotCopyPropagate) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t1", "t2", "t3" });
    frame.locals.push_back(integral(ir.strings, "x"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assign(n("t1"), n("t2")),
            ir::add(n("t2"), n("x"), n("t3")),
            ir::ret(n("t3")),
    }, std::move(frame)));

    ir = runIrPasses(std::move(ir), 0);

    EXPECT_THAT(toString(ir), HasSubstr("t3 := t2 + x"));
}

TEST(IrPasses, runIrPasses_atO1CopyPropagatesThenDropsDeadCopy) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t1", "t2", "t3" });
    frame.locals.push_back(integral(ir.strings, "x"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assign(n("t1"), n("t2")),
            ir::add(n("t2"), n("x"), n("t3")),
            ir::ret(n("t3")),
    }, std::move(frame)));

    ir = runIrPasses(std::move(ir), 1);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt3 := t1 + x\n"
            "\tRETURN t3\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_zeroCompareKnownZeroBecomesGoto) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("0"), n("t0")),
            ir::zeroCompare(n("t0")),
            ir::jump(n("end"), JumpCondition::IF_EQUAL),
            ir::inc(n("x")),
            ir::label(n("end")),
            ir::voidReturn(),
    }, ints(ir.strings, { "t0", "x" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt0 := 0\n"
            "\tGOTO end\n"
            "\tINC x\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_zeroCompareKnownNonzeroDropsJump) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t0")),
            ir::zeroCompare(n("t0")),
            ir::jump(n("end"), JumpCondition::IF_EQUAL),
            ir::inc(n("x")),
            ir::label(n("end")),
            ir::voidReturn(),
    }, ints(ir.strings, { "t0", "x" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt0 := 1\n"
            "\tINC x\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_valueCompareSignedBelowTaken) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::valueCompare(n("t1"), n("t2"), true),
            ir::jump(n("end"), JumpCondition::IF_BELOW, true),
            ir::inc(n("x")),
            ir::label(n("end")),
            ir::voidReturn(),
    }, ints(ir.strings, { "t1", "t2", "x" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt1 := 1\n"
            "\tt2 := 2\n"
            "\tGOTO end\n"
            "\tINC x\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_valueCompareUnsignedDoesNotUseSignedOrder) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("0xffffffff"), n("t1")),
            ir::assignConstant(n("0"), n("t2")),
            ir::valueCompare(n("t1"), n("t2"), false),
            ir::jump(n("end"), JumpCondition::IF_BELOW, false),
            ir::inc(n("x")),
            ir::label(n("end")),
            ir::voidReturn(),
    }, ints(ir.strings, { "t1", "t2", "x" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt1 := 0xffffffff\n"
            "\tt2 := 0\n"
            "\tINC x\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_skipsUnknownZeroCompare) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::zeroCompare(n("x")),
            ir::jump(n("end"), JumpCondition::IF_EQUAL),
            ir::inc(n("y")),
            ir::label(n("end")),
            ir::voidReturn(),
    }, ints(ir.strings, { "x", "y" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tCMP x, 0\n"
            "\tJE end\n"
            "\tINC y\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_skipsCompareWhenNotAdjacentToJump) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("0"), n("t0")),
            ir::zeroCompare(n("t0")),
            ir::inc(n("x")),
            ir::jump(n("end"), JumpCondition::IF_EQUAL),
            ir::label(n("end")),
            ir::voidReturn(),
    }, ints(ir.strings, { "t0", "x" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt0 := 0\n"
            "\tCMP t0, 0\n"
            "\tINC x\n"
            "\tJE end\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_keepsKnownAcrossSinglePredFallthroughLabel) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("0"), n("t0")),
            ir::label(n("L")),
            ir::zeroCompare(n("t0")),
            ir::jump(n("end"), JumpCondition::IF_EQUAL),
            ir::inc(n("x")),
            ir::label(n("end")),
            ir::voidReturn(),
    }, ints(ir.strings, { "t0", "x" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt0 := 0\n"
            "L:\n"
            "\tGOTO end\n"
            "\tINC x\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, foldConstants_clearsKnownAtJoinLabel) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("0"), n("t0")),
            ir::zeroCompare(n("x")),
            ir::jump(n("L"), JumpCondition::IF_EQUAL),
            ir::assignConstant(n("1"), n("t0")),
            ir::label(n("L")),
            ir::zeroCompare(n("t0")),
            ir::jump(n("end"), JumpCondition::IF_EQUAL),
            ir::inc(n("y")),
            ir::label(n("end")),
            ir::voidReturn(),
    }, ints(ir.strings, { "t0", "x", "y" })));

    foldConstants(ir.procedures.front(), ir.strings);

    EXPECT_THAT(toString(ir), HasSubstr("L:\n\tCMP t0, 0\n\tJE end\n"));
}

TEST(IrPasses, runIrPasses_atO1FoldsIfZeroDropsDeadArm) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t0" });
    frame.locals.push_back(integral(ir.strings, "x"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("0"), n("t0")),
            ir::zeroCompare(n("t0")),
            ir::jump(n("end"), JumpCondition::IF_EQUAL),
            ir::inc(n("x")),
            ir::label(n("end")),
            ir::voidReturn(),
    }, std::move(frame)));

    ir = runIrPasses(std::move(ir), 1);

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "end:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, runIrPasses_atO0KeepsIfZero) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("0"), n("t0")),
            ir::zeroCompare(n("t0")),
            ir::jump(n("end"), JumpCondition::IF_EQUAL),
            ir::inc(n("x")),
            ir::label(n("end")),
            ir::voidReturn(),
    }, ints(ir.strings, { "t0", "x" })));

    ir = runIrPasses(std::move(ir), 0);

    EXPECT_THAT(toString(ir), HasSubstr("CMP t0, 0"));
    EXPECT_THAT(toString(ir), HasSubstr("JE end"));
    EXPECT_THAT(toString(ir), HasSubstr("INC x"));
}

TEST(IrPasses, runIrPasses_atO1FoldsConstRelIf) {
    IntermediateRepresentation ir;
    IrN n { ir.strings };
    ProcedureFrame frame = exprTemps(ir.strings, { "t1", "t2", "r" });
    frame.locals.push_back(integral(ir.strings, "x"));
    ir.procedures.push_back(makeProc(ir.strings, "f", {
            ir::assignConstant(n("1"), n("t1")),
            ir::assignConstant(n("2"), n("t2")),
            ir::valueCompare(n("t1"), n("t2"), true),
            ir::jump(n("truthy"), JumpCondition::IF_BELOW, true),
            ir::assignConstant(n("0"), n("r")),
            ir::jump(n("join")),
            ir::label(n("truthy")),
            ir::assignConstant(n("1"), n("r")),
            ir::label(n("join")),
            ir::zeroCompare(n("r")),
            ir::jump(n("end"), JumpCondition::IF_EQUAL),
            ir::inc(n("x")),
            ir::label(n("end")),
            ir::voidReturn(),
    }, std::move(frame)));

    ir = runIrPasses(std::move(ir), 1);

    EXPECT_THAT(toString(ir), HasSubstr("INC x"));
    EXPECT_THAT(toString(ir), Not(HasSubstr("CMP")));
    EXPECT_THAT(toString(ir), Not(HasSubstr("JE")));
    EXPECT_THAT(toString(ir), Not(HasSubstr("JB")));
}

} // namespace
