#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/quadruples/BuiltinOp.h"

using namespace codegen;

// Contract for the BuiltinOp quadruple used by real-frontend lowering of
// __builtin_bswap* / __builtin_ctz*.
TEST(BuiltinOp, encodesKindOperandAndResult) {
    BuiltinOp op { BuiltinOp::Kind::Bswap32, "arg0", "res0" };
    EXPECT_EQ(op.getKind(), BuiltinOp::Kind::Bswap32);
    EXPECT_EQ(op.getOperand(), "arg0");
    EXPECT_EQ(op.getResult(), "res0");
}

TEST(BuiltinOp, distinguishesCtzAndBswapKinds) {
    BuiltinOp bswap { BuiltinOp::Kind::Bswap16, "x", "y" };
    BuiltinOp ctz { BuiltinOp::Kind::Ctz, "x", "y" };
    EXPECT_NE(bswap.getKind(), ctz.getKind());
    EXPECT_EQ(ctz.getKind(), BuiltinOp::Kind::Ctz);
}
