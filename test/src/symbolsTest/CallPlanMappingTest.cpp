#include "gtest/gtest.h"

#include <string>
#include <variant>

#include "builtins/BuiltinRegistry.h"
#include "symbols/NodeRef.h"
#include "symbols/BuiltinPlan.h"
#include "symbols/CallPlan.h"
#include "types/Type.h"

TEST(CallPlanMapping, constantPIsBuiltinPlan) {
    auto d = builtins::lookupBuiltin("__builtin_constant_p");
    ASSERT_TRUE(d.has_value());
    const auto* simple = std::get_if<builtins::SimpleBuiltin>(&d->kind);
    ASSERT_NE(simple, nullptr);
    EXPECT_TRUE(symbols::get_if<symbols::ConstantZeroPlan>(&simple->plan));
}

TEST(CallPlanMapping, bswapIsRegistryBuiltinWithBswapPlan) {
    auto b16 = builtins::lookupBuiltin("__builtin_bswap16");
    auto b32 = builtins::lookupBuiltin("__builtin_bswap32");
    auto b64 = builtins::lookupBuiltin("__builtin_bswap64");
    ASSERT_TRUE(b16.has_value());
    ASSERT_TRUE(b32.has_value());
    ASSERT_TRUE(b64.has_value());
    const auto* s16 = std::get_if<builtins::SimpleBuiltin>(&b16->kind);
    const auto* s32 = std::get_if<builtins::SimpleBuiltin>(&b32->kind);
    const auto* s64 = std::get_if<builtins::SimpleBuiltin>(&b64->kind);
    ASSERT_NE(s16, nullptr);
    ASSERT_NE(s32, nullptr);
    ASSERT_NE(s64, nullptr);
    const auto* p16 = symbols::get_if<symbols::BswapPlan>(&s16->plan);
    const auto* p32 = symbols::get_if<symbols::BswapPlan>(&s32->plan);
    const auto* p64 = symbols::get_if<symbols::BswapPlan>(&s64->plan);
    ASSERT_NE(p16, nullptr);
    ASSERT_NE(p32, nullptr);
    ASSERT_NE(p64, nullptr);
    EXPECT_EQ(p16->widthBytes, 2);
    EXPECT_EQ(p32->widthBytes, 4);
    EXPECT_EQ(p64->widthBytes, 8);
}

TEST(CallPlanMapping, allocaIsSyntheticDirectCallMalloc) {
    auto d = builtins::lookupBuiltin("__builtin_alloca");
    ASSERT_TRUE(d.has_value());
    const auto* syn = std::get_if<builtins::SyntheticCall>(&d->kind);
    ASSERT_NE(syn, nullptr);
    EXPECT_EQ(syn->callee, "malloc");
}

TEST(CallPlanMapping, vaArgIsTypeNameReturnNotADummyPlan) {
    auto va = builtins::lookupBuiltin("__builtin_va_arg");
    ASSERT_TRUE(va.has_value());
    EXPECT_TRUE(std::holds_alternative<builtins::TypeNameReturn>(va->kind));
}

TEST(CallPlanMapping, offsetofIsNotAFunctionCallBuiltin) {
    EXPECT_FALSE(builtins::lookupBuiltin("__builtin_offsetof").has_value());
}

TEST(CallPlan, enumDirectAndIndirect) {
    EXPECT_FALSE(symbols::isIndirectCall(symbols::CallPlan::Direct));
    EXPECT_TRUE(symbols::isIndirectCall(symbols::CallPlan::Indirect));
    EXPECT_TRUE(symbols::isDirectCall(symbols::CallPlan::Direct));
    EXPECT_FALSE(symbols::isDirectCall(symbols::CallPlan::Indirect));
}
