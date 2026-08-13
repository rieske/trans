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
    symbols::ValueEntry v { "x", type::signedInteger(), ctx(), 0 };
    EXPECT_EQ(v.getName(), "x");
    EXPECT_TRUE(v.getType().isPrimitive());
    EXPECT_EQ(v.getIndex(), 0);
    EXPECT_EQ(v.globalInitializer(), nullptr);
}

TEST(Symbols, globalInitializerIsClosedVariant) {
    symbols::ValueEntry v { "g", type::signedInteger(), ctx(), 0, symbols::Storage::Global };
    v.setGlobalInitializer(symbols::ConstantInit { 42 });
    ASSERT_NE(v.globalInitializer(), nullptr);
    ASSERT_TRUE(std::holds_alternative<symbols::ConstantInit>(*v.globalInitializer()));
    EXPECT_EQ(std::get<symbols::ConstantInit>(*v.globalInitializer()).value, 42);

    v.setGlobalInitializer(symbols::AddressInit { "other", 0 });
    ASSERT_TRUE(std::holds_alternative<symbols::AddressInit>(*v.globalInitializer()));
    EXPECT_EQ(std::get<symbols::AddressInit>(*v.globalInitializer()).symbol, "other");

    v.setGlobalInitializer(symbols::MultiWordInit { {
            symbols::ConstantInit { 1 }, symbols::ConstantInit { 2 } } });
    ASSERT_TRUE(std::holds_alternative<symbols::MultiWordInit>(*v.globalInitializer()));
    EXPECT_EQ(std::get<symbols::MultiWordInit>(*v.globalInitializer()).words.size(), 2u);
}

TEST(Symbols, promoteExternToDefinitionAndDefiningInitializer) {
    symbols::ValueEntry v { "e", type::signedInteger(), ctx(), 0, symbols::Storage::Extern };
    EXPECT_FALSE(v.hasDefiningInitializer());
    v.promoteExternToDefinition();
    EXPECT_FALSE(v.isExtern());
    EXPECT_TRUE(v.isGlobal());
    v.markDefiningInitializer();
    EXPECT_TRUE(v.hasDefiningInitializer());
}

TEST(Symbols, storageEnumStaticIsGlobalNotExtern) {
    symbols::ValueEntry v { "s", type::signedInteger(), ctx(), 0, symbols::Storage::Static };
    EXPECT_TRUE(v.isGlobal());
    EXPECT_TRUE(v.isStatic());
    EXPECT_FALSE(v.isExtern());
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
