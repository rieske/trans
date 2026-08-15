#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "semantic_analyzer/SymbolTable.h"
#include "symbols/StaticInit.h"
#include "symbols/ValueEntry.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

#include <variant>

namespace {

using namespace testing;
using namespace semantic_analyzer;

// Empty formal names (abstract parameters) must still produce one symbol-table
// argument slot per declared parameter. Silent drop of the second empty name
// collapses the callee ABI while the function type arity stays correct.

TEST(SymbolTable, abstractArgumentNamesPreserveArity) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), { type::signedInteger(), type::signedInteger() });
    table.insertFunction("add", functionType.getFunction(), ctx);

    table.startFunction("add", { "", "" });

    EXPECT_THAT(table.getCurrentScopeArguments().size(), Eq(2u));
}

TEST(SymbolTable, unnamedStaticObjectIsDataHome) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    const ValueEntry home = table.createUnnamedStaticObject(type::signedInteger(), ctx);

    EXPECT_TRUE(home.isGlobal());
    EXPECT_TRUE(home.isStatic());
    EXPECT_THAT(home.getName(), StartsWith("L$cl"));
    EXPECT_TRUE(home.getType().equivalentTo(type::signedInteger()));

    table.setStaticInit(home.getName(), symbols::asDataWords(symbols::StaticInteger { 7 }));

    const std::vector<ValueEntry> homes = table.getDataHomes();
    ASSERT_THAT(homes.size(), Eq(1u));
    EXPECT_THAT(homes.front().getName(), Eq(home.getName()));
    ASSERT_THAT(homes.front().staticInit().size(), Eq(1u));
}

TEST(SymbolTable, unnamedStaticObjectsHaveDistinctNames) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    const ValueEntry first = table.createUnnamedStaticObject(type::signedInteger(), ctx);
    const ValueEntry second = table.createUnnamedStaticObject(
            type::array(type::signedInteger(), 3), ctx);

    EXPECT_THAT(first.getName(), Ne(second.getName()));
    EXPECT_THAT(table.getDataHomes().size(), Eq(2u));
}

TEST(SymbolTable, fileScopeTemporaryIsNotADataHome) {
    SymbolTable table;
    table.createTemporarySymbol(type::signedInteger());
    EXPECT_TRUE(table.getDataHomes().empty());
}

const symbols::StaticWord* firstWord(const ValueEntry& home) {
    if (home.staticInit().empty()) {
        return nullptr;
    }
    return std::get_if<symbols::StaticWord>(&home.staticInit().front());
}

const ValueEntry* dataHomeNamed(const std::vector<ValueEntry>& homes, const std::string& name) {
    for (const auto& home : homes) {
        if (home.getName() == name) {
            return &home;
        }
    }
    return nullptr;
}

TEST(SymbolTable, staticLocalInitDoesNotClobberSameNamedGlobal) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Global);
    table.setStaticInit("g", symbols::asDataWords(symbols::StaticInteger { 1 }));

    table.insertFunction("f", type::function(type::signedInteger(), {}).getFunction(), ctx);
    table.startFunction("f", {});
    table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Static);
    table.setStaticInit("g", symbols::asDataWords(symbols::StaticInteger { 2 }));
    table.endFunction();

    const std::vector<ValueEntry> homes = table.getDataHomes();
    const ValueEntry* global = dataHomeNamed(homes, "g");
    ASSERT_NE(global, nullptr);
    const auto* globalWord = firstWord(*global);
    ASSERT_NE(globalWord, nullptr);
    EXPECT_THAT(globalWord->bits, Eq(1ull));

    const ValueEntry* local = nullptr;
    for (const auto& home : homes) {
        if (home.isStatic() && home.getName() != "g") {
            local = &home;
            break;
        }
    }
    ASSERT_NE(local, nullptr);
    const auto* localWord = firstWord(*local);
    ASSERT_NE(localWord, nullptr);
    EXPECT_THAT(localWord->bits, Eq(2ull));
}

TEST(SymbolTable, unnamedStaticInitFromInsideFunctionUpdatesTuHome) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    const ValueEntry home = table.createUnnamedStaticObject(type::signedInteger(), ctx);

    table.insertFunction("f", type::function(type::signedInteger(), {}).getFunction(), ctx);
    table.startFunction("f", {});
    table.setStaticInit(home.getName(), symbols::asDataWords(symbols::StaticInteger { 9 }));
    table.endFunction();

    const std::vector<ValueEntry> homes = table.getDataHomes();
    const ValueEntry* found = dataHomeNamed(homes, home.getName());
    ASSERT_NE(found, nullptr);
    const auto* word = firstWord(*found);
    ASSERT_NE(word, nullptr);
    EXPECT_THAT(word->bits, Eq(9ull));
}

} // namespace
