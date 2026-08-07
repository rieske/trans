#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Instruction.h"
#include "codegen/IrPasses.h"
#include "codegen/Value.h"

namespace {

using namespace testing;
using namespace codegen;

Procedure makeProc(std::string name, std::vector<Instruction> body,
        ProcedureFrame frame = {}) {
    Procedure p;
    p.name = std::move(name);
    p.frame = std::move(frame);
    p.body = std::move(body);
    return p;
}

TEST(IrPasses, sealProcedures_padsFallOffEnd) {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("f", { ir::assignConstant("1", "t0") }));

    ir = sealProcedures(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt0 := 1\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, sealProcedures_doesNotPadExplicitReturn) {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("f",
            { ir::assignConstant("0", "t0"), ir::ret("t0") },
            ProcedureFrame { { codegen::Value { "t0", 0, codegen::Type::INTEGRAL, 8 } }, {} }));

    ir = sealProcedures(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tt0 := 0\n"
            "\tRETURN t0\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, sealProcedures_padsAfterFallthroughToExitLabel) {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("f", {
            ir::zeroCompare("x"),
            ir::jump("end", JumpCondition::IF_EQUAL),
            ir::inc("y"),
            ir::label("end"),
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

TEST(IrPasses, eliminateJumpToNext_removesRedundantGoto) {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("f", {
            ir::jump("L"),
            ir::label("L"),
            ir::inc("x"),
    }));

    ir = eliminateJumpToNext(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "L:\n"
            "\tINC x\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, eliminateJumpToNext_keepsConditionalAndNonAdjacent) {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("f", {
            ir::jump("L", JumpCondition::IF_EQUAL),
            ir::label("L"),
            ir::jump("M"),
            ir::inc("x"),
            ir::label("M"),
    }));

    ir = eliminateJumpToNext(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "\tJE L\n"
            "L:\n"
            "\tGOTO M\n"
            "\tINC x\n"
            "M:\n"
            "ENDPROC f\n"));
}

TEST(IrPasses, runIrPasses_composesSealAndPeephole) {
    IntermediateRepresentation ir;
    ir.procedures.push_back(makeProc("f", {
            ir::jump("done"),
            ir::label("done"),
    }));

    ir = runIrPasses(std::move(ir));

    EXPECT_THAT(toString(ir), StrEq(
            "PROC f\n"
            "done:\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

} // namespace
