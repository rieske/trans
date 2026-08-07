#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/Instruction.h"
#include "symbols/BuiltinPlan.h"

using namespace codegen;

// Contract for the BuiltinOp instruction used by real-frontend lowering of
// __builtin_bswap* / __builtin_ctz*.
TEST(BuiltinOp, encodesKindOperandAndResult) {
    Instruction op = ir::builtinOp(symbols::BuiltinOpKind::Bswap32, "arg0", "res0");
    EXPECT_EQ(op.builtinKind, symbols::BuiltinOpKind::Bswap32);
    EXPECT_EQ(op.arg0, "arg0");
    EXPECT_EQ(op.result, "res0");
    EXPECT_EQ(op.op, Op::BuiltinOp);
}

TEST(BuiltinOp, distinguishesCtzAndBswapKinds) {
    Instruction bswap = ir::builtinOp(symbols::BuiltinOpKind::Bswap16, "x", "y");
    Instruction ctz = ir::builtinOp(symbols::BuiltinOpKind::Ctz, "x", "y");
    EXPECT_NE(bswap.builtinKind, ctz.builtinKind);
    EXPECT_EQ(ctz.builtinKind, symbols::BuiltinOpKind::Ctz);
}
