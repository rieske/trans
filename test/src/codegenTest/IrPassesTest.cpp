#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Instruction.h"
#include "codegen/IrPasses.h"
#include "codegen/Value.h"

#include <string_view>

#include "codegen/IrStringTable.h"

#include <iterator>

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

} // namespace
