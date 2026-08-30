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
using symbols::ValueEntry;

void startIntFunction(SymbolTable& table, const char* name = "f") {
    translation_unit::Context ctx { "t.c", 1 };
    table.insertFunction(name, type::function(type::signedInteger(), {}).getFunction(), ctx);
    table.startFunction(name, {});
}

// Empty formal names (abstract parameters) must still produce one symbol-table
// argument slot per declared parameter. Silent drop of the second empty name
// collapses the callee ABI while the function type arity stays correct.

TEST(SymbolTable, abstractArgumentNamesPreserveArity) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), { type::signedInteger(), type::signedInteger() });
    table.insertFunction("add", functionType.getFunction(), ctx);

    table.startFunction("add", { "", "" });

    const auto arguments = table.getCurrentScopeArguments();
    ASSERT_THAT(arguments.size(), Eq(2u));
    EXPECT_THAT(arguments[0].getName(), Eq("$s1__arg0"));
    EXPECT_THAT(arguments[1].getName(), Eq("$s1__arg1"));
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

int countHomesNamed(const std::vector<ValueEntry>& homes, const std::string& name) {
    int n = 0;
    for (const auto& home : homes) {
        if (home.getName() == name) {
            ++n;
        }
    }
    return n;
}

// Block-scope extern has external linkage: it is the file-scope object, not a
// second data home.
TEST(SymbolTable, localExternDoesNotDuplicateFileScopeDataHome) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Global);

    table.insertFunction("f", type::function(type::signedInteger(), {}).getFunction(), ctx);
    table.startFunction("f", {});
    table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Extern);
    table.endFunction();

    const std::vector<ValueEntry> homes = table.getDataHomes();
    EXPECT_THAT(countHomesNamed(homes, "g"), Eq(1));
    const ValueEntry* g = dataHomeNamed(homes, "g");
    ASSERT_NE(g, nullptr);
    EXPECT_FALSE(g->isExtern());
}

// No prior file-scope declaration: the local extern *is* the TU data home.
TEST(SymbolTable, localExternCreatesFileScopeHomeWhenMissing) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    table.insertFunction("f", type::function(type::signedInteger(), {}).getFunction(), ctx);
    table.startFunction("f", {});
    table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Extern);
    table.endFunction();

    EXPECT_TRUE(table.hasGlobalVariable("g"));
    const ValueEntry* g = dataHomeNamed(table.getDataHomes(), "g");
    ASSERT_NE(g, nullptr);
    EXPECT_TRUE(g->isExtern());
}

// Later file-scope definition is the same object (extern -> definition).
TEST(SymbolTable, fileScopeDefinitionPromotesEarlierLocalExtern) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    table.insertFunction("f", type::function(type::signedInteger(), {}).getFunction(), ctx);
    table.startFunction("f", {});
    table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Extern);
    table.endFunction();

    EXPECT_THAT(table.bindFileScopeObject("g", type::signedInteger(), ctx,
            symbols::Storage::Global, true), Eq(ObjectBind::Bound));

    const ValueEntry* g = dataHomeNamed(table.getDataHomes(), "g");
    ASSERT_NE(g, nullptr);
    EXPECT_FALSE(g->isExtern());
    EXPECT_THAT(countHomesNamed(table.getDataHomes(), "g"), Eq(1));
}

// Two functions declaring `extern int g` still name one object.
TEST(SymbolTable, localExternsOfSameNameShareOneDataHome) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    const auto fnType = type::function(type::signedInteger(), {}).getFunction();
    table.insertFunction("f", fnType, ctx);
    table.startFunction("f", {});
    table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Extern);
    table.endFunction();

    table.insertFunction("h", fnType, ctx);
    table.startFunction("h", {});
    table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Extern);
    table.endFunction();

    const std::vector<ValueEntry> homes = table.getDataHomes();
    EXPECT_THAT(countHomesNamed(homes, "g"), Eq(1));
    const ValueEntry* g = dataHomeNamed(homes, "g");
    ASSERT_NE(g, nullptr);
    EXPECT_TRUE(g->isExtern());
}

TEST(SymbolTable, localExternTypeConflictDoesNotInsert) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Global);

    table.insertFunction("f", type::function(type::signedInteger(), {}).getFunction(), ctx);
    table.startFunction("f", {});
    EXPECT_FALSE(table.insertSymbol("g", type::pointer(type::signedInteger()), ctx,
            symbols::Storage::Extern));
    table.endFunction();

    const ValueEntry* g = dataHomeNamed(table.getDataHomes(), "g");
    ASSERT_NE(g, nullptr);
    EXPECT_FALSE(g->getType().isPointer());
}

TEST(SymbolTable, localExternConflictsWithFileScopeFunction) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    table.insertFunction("g", type::function(type::voidType(), {}).getFunction(), ctx);
    table.insertFunction("f", type::function(type::signedInteger(), {}).getFunction(), ctx);
    table.startFunction("f", {});
    EXPECT_FALSE(table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Extern));
    table.endFunction();

    EXPECT_FALSE(table.hasGlobalVariable("g"));
}

TEST(SymbolTable, automaticLocalUsesDollarSKey) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    startIntFunction(table);
    ASSERT_TRUE(table.insertSymbol("x", type::signedInteger(), ctx));

    EXPECT_THAT(table.lookup("x").getName(), Eq("$s1x"));
    EXPECT_THAT(table.getCurrentScopeSymbols(), Contains(Key("$s1x")));
}

TEST(SymbolTable, siblingBlocksGetDistinctDollarSKeys) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    startIntFunction(table);

    table.enterBlockScope();
    ASSERT_TRUE(table.insertSymbol("x", type::signedInteger(), ctx));
    EXPECT_THAT(table.lookup("x").getName(), Eq("$s2x"));
    table.exitBlockScope();

    table.enterBlockScope();
    ASSERT_TRUE(table.insertSymbol("x", type::signedInteger(), ctx));
    EXPECT_THAT(table.lookup("x").getName(), Eq("$s3x"));
    table.exitBlockScope();

    const auto symbols = table.getCurrentScopeSymbols();
    EXPECT_THAT(symbols, Contains(Key("$s2x")));
    EXPECT_THAT(symbols, Contains(Key("$s3x")));
}

TEST(SymbolTable, parameterObjectNameIsDollarSPrefixed) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    const auto functionType = type::function(
            type::signedInteger(), { type::signedInteger(), type::signedInteger() });
    table.insertFunction("add", functionType.getFunction(), ctx);
    table.startFunction("add", { "lhs", "rhs" });

    const auto arguments = table.getCurrentScopeArguments();
    ASSERT_THAT(arguments.size(), Eq(2u));
    EXPECT_THAT(arguments[0].getName(), Eq("$s1lhs"));
    EXPECT_THAT(arguments[1].getName(), Eq("$s1rhs"));
    EXPECT_THAT(table.lookup("lhs").getName(), Eq("$s1lhs"));
    EXPECT_THAT(table.lookup("rhs").getName(), Eq("$s1rhs"));
}

TEST(SymbolTable, siblingBlocksDoNotReuseFrameSlots) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    startIntFunction(table);

    table.enterBlockScope();
    ASSERT_TRUE(table.insertSymbol("a", type::signedInteger(), ctx));
    const int firstIndex = table.lookup("a").getIndex();
    table.exitBlockScope();

    table.enterBlockScope();
    ASSERT_TRUE(table.insertSymbol("b", type::signedInteger(), ctx));
    const int secondIndex = table.lookup("b").getIndex();
    table.exitBlockScope();

    EXPECT_THAT(secondIndex, Gt(firstIndex));
}

TEST(SymbolTable, staticLocalObjectNameIsLDollarSt) {
    SymbolTable table;
    translation_unit::Context ctx { "t.c", 1 };
    startIntFunction(table);
    ASSERT_TRUE(table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Static));

    EXPECT_THAT(table.lookup("g").getName(), Eq("L$st1_g"));
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
