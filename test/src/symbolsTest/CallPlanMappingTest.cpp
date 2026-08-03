#include "gtest/gtest.h"

#include <algorithm>
#include <string>
#include <vector>
#include <variant>

#include "builtins/BuiltinRegistry.h"
#include "symbols/NodeRef.h"
#include "symbols/BuiltinPlan.h"
#include "symbols/CallPlan.h"
#include "types/Type.h"

TEST(CallPlanMapping, constantPIsNotAFunctionCallBuiltin) {
    EXPECT_FALSE(builtins::lookupBuiltin("__builtin_constant_p").has_value());
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

TEST(CallPlanMapping, allocaIsAllocaPlan) {
    auto d = builtins::lookupBuiltin("__builtin_alloca");
    ASSERT_TRUE(d.has_value());
    const auto* simple = std::get_if<builtins::SimpleBuiltin>(&d->kind);
    ASSERT_NE(simple, nullptr);
    EXPECT_TRUE(symbols::get_if<symbols::AllocaPlan>(&simple->plan));
}

TEST(CallPlanMapping, ctzCarriesOperandWidth) {
    auto z = builtins::lookupBuiltin("__builtin_ctz");
    auto zl = builtins::lookupBuiltin("__builtin_ctzl");
    auto zll = builtins::lookupBuiltin("__builtin_ctzll");
    ASSERT_TRUE(z.has_value());
    ASSERT_TRUE(zl.has_value());
    ASSERT_TRUE(zll.has_value());
    const auto* sz = std::get_if<builtins::SimpleBuiltin>(&z->kind);
    const auto* sl = std::get_if<builtins::SimpleBuiltin>(&zl->kind);
    const auto* sll = std::get_if<builtins::SimpleBuiltin>(&zll->kind);
    ASSERT_NE(sz, nullptr);
    ASSERT_NE(sl, nullptr);
    ASSERT_NE(sll, nullptr);
    const auto* pz = symbols::get_if<symbols::CtzPlan>(&sz->plan);
    const auto* pl = symbols::get_if<symbols::CtzPlan>(&sl->plan);
    const auto* pll = symbols::get_if<symbols::CtzPlan>(&sll->plan);
    ASSERT_NE(pz, nullptr);
    ASSERT_NE(pl, nullptr);
    ASSERT_NE(pll, nullptr);
    EXPECT_EQ(pz->widthBytes, 4);
    EXPECT_EQ(pl->widthBytes, 8);
    EXPECT_EQ(pll->widthBytes, 8);
}

TEST(CallPlanMapping, vaArgIsTypeNameReturnNotADummyPlan) {
    auto va = builtins::lookupBuiltin("__builtin_va_arg");
    ASSERT_TRUE(va.has_value());
    EXPECT_TRUE(std::holds_alternative<builtins::TypeNameReturn>(va->kind));
}

TEST(CallPlanMapping, offsetofIsNotAFunctionCallBuiltin) {
    EXPECT_FALSE(builtins::lookupBuiltin("__builtin_offsetof").has_value());
}

TEST(CallPlanMapping, designatorBuiltinsAreBswapCtzAlloca) {
    std::vector<std::string> names;
    builtins::forEachDesignatorBuiltin([&](const char* name, const builtins::BuiltinDescriptor&) {
        names.emplace_back(name);
    });
    std::sort(names.begin(), names.end());
    const std::vector<std::string> expected {
            "__builtin_alloca",
            "__builtin_bswap16",
            "__builtin_bswap32",
            "__builtin_bswap64",
            "__builtin_ctz",
            "__builtin_ctzl",
            "__builtin_ctzll",
    };
    EXPECT_EQ(names, expected);
    EXPECT_TRUE(std::find(names.begin(), names.end(), "__builtin_va_start") == names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "__builtin_constant_p") == names.end());
}

TEST(CallPlan, enumDirectAndIndirect) {
    EXPECT_FALSE(symbols::isIndirectCall(symbols::CallPlan::Direct));
    EXPECT_TRUE(symbols::isIndirectCall(symbols::CallPlan::Indirect));
    EXPECT_TRUE(symbols::isDirectCall(symbols::CallPlan::Direct));
    EXPECT_FALSE(symbols::isDirectCall(symbols::CallPlan::Indirect));
}
