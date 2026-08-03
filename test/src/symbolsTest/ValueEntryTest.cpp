#include "gtest/gtest.h"

#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "symbols/FunctionEntry.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

#include <variant>

namespace {

translation_unit::Context ctx() { return { "t", 1 }; }

TEST(Symbols, valueEntryHoldsTypeAndName) {
    symbols::ValueEntry v { "x", type::signedInteger(), false, ctx(), 0 };
    EXPECT_EQ(v.getName(), "x");
    EXPECT_TRUE(v.getType().isPrimitive());
    EXPECT_EQ(v.getIndex(), 0);
    EXPECT_EQ(v.globalInitializer(), nullptr);
}

TEST(Symbols, globalInitializerIsClosedVariant) {
    symbols::ValueEntry v { "g", type::signedInteger(), false, ctx(), 0, true };
    v.setGlobalInitializer(symbols::ConstantInit { 42 });
    ASSERT_NE(v.globalInitializer(), nullptr);
    ASSERT_TRUE(std::holds_alternative<symbols::ConstantInit>(*v.globalInitializer()));
    EXPECT_EQ(std::get<symbols::ConstantInit>(*v.globalInitializer()).value, 42);

    v.setGlobalInitializer(symbols::StringInit { "hi" });
    ASSERT_TRUE(std::holds_alternative<symbols::StringInit>(*v.globalInitializer()));
    EXPECT_EQ(std::get<symbols::StringInit>(*v.globalInitializer()).value, "hi");

    v.setGlobalInitializer(symbols::AddressInit { "other" });
    ASSERT_TRUE(std::holds_alternative<symbols::AddressInit>(*v.globalInitializer()));
    EXPECT_EQ(std::get<symbols::AddressInit>(*v.globalInitializer()).symbolName, "other");

    v.setGlobalInitializer(symbols::MultiWordInit { { "1", "2" } });
    ASSERT_TRUE(std::holds_alternative<symbols::MultiWordInit>(*v.globalInitializer()));
    EXPECT_EQ(std::get<symbols::MultiWordInit>(*v.globalInitializer()).words.size(), 2u);
}

TEST(Symbols, convenienceWritersForwardToGlobalInitializer) {
    symbols::ValueEntry v { "g", type::signedInteger(), false, ctx(), 0, true };
    v.setConstantInitializer(7);
    ASSERT_TRUE(std::holds_alternative<symbols::ConstantInit>(*v.globalInitializer()));
    v.setStringInitializer("s");
    ASSERT_TRUE(std::holds_alternative<symbols::StringInit>(*v.globalInitializer()));
}

TEST(Symbols, labelEntryName) {
    symbols::LabelEntry lab { "L1" };
    EXPECT_EQ(lab.getName(), "L1");
}

TEST(Symbols, functionEntryArity) {
    auto fty = type::function(type::signedInteger(), { type::signedInteger() });
    symbols::FunctionEntry f { "f", fty.getFunction(), ctx() };
    EXPECT_EQ(f.getName(), "f");
    EXPECT_EQ(f.argumentCount(), 1u);
}

} // namespace
