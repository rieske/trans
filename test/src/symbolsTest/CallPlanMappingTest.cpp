#include "gtest/gtest.h"

#include <string>

#include "builtins/BuiltinRegistry.h"
#include "symbols/NodeRef.h"
#include "symbols/BuiltinPlan.h"
#include "symbols/CallPlan.h"
#include "types/Type.h"

TEST(CallPlanMapping, constantPIsBuiltinPlan) {
    auto d = builtins::lookupBuiltin("__builtin_constant_p", nullptr);
    ASSERT_TRUE(d.has_value());
    ASSERT_TRUE(d->builtinPlan.has_value());
    EXPECT_TRUE(symbols::get_if<symbols::ConstantZeroPlan>(*d->builtinPlan));
    EXPECT_FALSE(d->syntheticCallee.has_value());
}

TEST(CallPlanMapping, bswap32IsBuiltinOpWithOpKind) {
    auto d = builtins::lookupBuiltin("__builtin_bswap32", nullptr);
    ASSERT_TRUE(d.has_value());
    ASSERT_TRUE(d->builtinPlan.has_value());
    const auto* bop = symbols::get_if<symbols::BuiltinOpPlan>(*d->builtinPlan);
    ASSERT_NE(bop, nullptr);
    EXPECT_EQ(bop->opKind, symbols::BuiltinOpKind::Bswap32);
}

TEST(CallPlanMapping, allocaIsSyntheticDirectCallMalloc) {
    auto d = builtins::lookupBuiltin("__builtin_alloca", nullptr);
    ASSERT_TRUE(d.has_value());
    ASSERT_TRUE(d->syntheticCallee.has_value());
    EXPECT_EQ(*d->syntheticCallee, "malloc");
    EXPECT_FALSE(d->builtinPlan.has_value());
}

TEST(CallPlanMapping, vaArgUsesResultType) {
    type::Type longTy = type::signedLong();
    auto d = builtins::lookupBuiltin("__builtin_va_arg", &longTy);
    ASSERT_TRUE(d.has_value());
    EXPECT_TRUE(d->returnType.equivalentTo(longTy));
    ASSERT_TRUE(d->builtinPlan.has_value());
    EXPECT_TRUE(symbols::get_if<symbols::VaArgPlan>(*d->builtinPlan));
}

TEST(CallPlan, enumDirectAndIndirect) {
    EXPECT_FALSE(symbols::isIndirectCall(symbols::CallPlan::Direct));
    EXPECT_TRUE(symbols::isIndirectCall(symbols::CallPlan::Indirect));
    EXPECT_TRUE(symbols::isDirectCall(symbols::CallPlan::Direct));
    EXPECT_FALSE(symbols::isDirectCall(symbols::CallPlan::Indirect));
}
