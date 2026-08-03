#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "semantic_analyzer/SymbolTable.h"
#include "symbols/GlobalInitializer.h"
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

    table.setGlobalInitializer(home.getName(), symbols::ConstantInit { 7 });

    const std::vector<ValueEntry> homes = table.getDataHomes();
    ASSERT_THAT(homes.size(), Eq(1u));
    EXPECT_THAT(homes.front().getName(), Eq(home.getName()));
    const auto* init = homes.front().globalInitializer();
    ASSERT_NE(init, nullptr);
    const auto* constant = std::get_if<symbols::ConstantInit>(init);
    ASSERT_NE(constant, nullptr);
    EXPECT_THAT(constant->value, Eq(7));
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

const symbols::ConstantInit* firstConstant(const ValueEntry& home) {
    const auto* init = home.globalInitializer();
    if (!init) {
        return nullptr;
    }
    return std::get_if<symbols::ConstantInit>(init);
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
    table.setGlobalInitializer("g", symbols::ConstantInit { 1 });

    table.insertFunction("f", type::function(type::signedInteger(), {}).getFunction(), ctx);
    table.startFunction("f", {});
    table.insertSymbol("g", type::signedInteger(), ctx, symbols::Storage::Static);
    table.setGlobalInitializer("g", symbols::ConstantInit { 2 });
    table.endFunction();

    const std::vector<ValueEntry> homes = table.getDataHomes();
    const ValueEntry* global = dataHomeNamed(homes, "g");
    ASSERT_NE(global, nullptr);
    const auto* globalWord = firstConstant(*global);
    ASSERT_NE(globalWord, nullptr);
    EXPECT_THAT(globalWord->value, Eq(1));

    const ValueEntry* local = nullptr;
    for (const auto& home : homes) {
        if (home.isStatic() && home.getName() != "g") {
            local = &home;
            break;
        }
    }
    ASSERT_NE(local, nullptr);
    const auto* localWord = firstConstant(*local);
    ASSERT_NE(localWord, nullptr);
    EXPECT_THAT(localWord->value, Eq(2));
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
    table.setGlobalInitializer(home.getName(), symbols::ConstantInit { 9 });
    table.endFunction();

    const std::vector<ValueEntry> homes = table.getDataHomes();
    const ValueEntry* found = dataHomeNamed(homes, home.getName());
    ASSERT_NE(found, nullptr);
    const auto* word = firstConstant(*found);
    ASSERT_NE(word, nullptr);
    EXPECT_THAT(word->value, Eq(9));
}

// Functions live in the function table, not as global objects. Lookup uses hasFunction.
TEST(SymbolTable, insertFunctionDoesNotCreateGlobalObject) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), {});
    table.insertFunction("f", functionType.getFunction(), ctx);

    EXPECT_TRUE(table.hasFunction("f"));
    EXPECT_FALSE(table.hasGlobalVariable("f"));
    EXPECT_FALSE(table.hasSymbol("f"));
}

TEST(SymbolTable, fileScopeObjectRejectedWhenFunctionExists) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), {});
    table.insertFunction("f", functionType.getFunction(), ctx);
    EXPECT_FALSE(table.insertSymbol("f", type::signedInteger(), ctx, symbols::Storage::Global));
    EXPECT_FALSE(table.hasGlobalVariable("f"));
}

TEST(SymbolTable, insertExternRejectsRegisteredFunction) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::voidType(), {});
    table.insertFunction("later", functionType.getFunction(), ctx);
    EXPECT_TRUE(table.hasFunction("later"));
    table.startFunction("later", {});
    EXPECT_FALSE(table.insertSymbol("later", type::signedInteger(), ctx, symbols::Storage::Extern));
    table.endFunction();
}

TEST(SymbolTable, insertExternTypeMismatchRejected) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    ASSERT_TRUE(table.insertSymbol("x", type::signedCharacter(), ctx, symbols::Storage::Global));
    EXPECT_FALSE(table.insertSymbol("x", type::signedInteger(), ctx, symbols::Storage::Extern));
}

TEST(SymbolTable, insertSymbolAtFileScopeIsInsertOnce) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    ASSERT_TRUE(table.insertSymbol("x", type::signedInteger(), ctx, symbols::Storage::Global));
    EXPECT_FALSE(table.insertSymbol("x", type::signedInteger(), ctx, symbols::Storage::Global));
    EXPECT_FALSE(table.lookup("x").hasDefiningInitializer());
}

TEST(SymbolTable, markFunctionDefinedSurvivesUpdate) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), {});
    table.insertFunction("f", functionType.getFunction(), ctx, true);
    EXPECT_FALSE(table.isFunctionDefined("f"));
    table.markFunctionDefined("f");
    EXPECT_TRUE(table.isFunctionDefined("f"));
    auto updated = type::function(type::voidType(), {});
    table.updateFunction("f", updated.getFunction(), { "u.c", 2 });
    EXPECT_TRUE(table.isFunctionDefined("f"));
    EXPECT_TRUE(table.findFunction("f").hasInternalLinkage());
    EXPECT_EQ(table.findFunction("f").getContext().getSourceName(), "u.c");
    EXPECT_TRUE(table.findFunction("f").returnType().isVoid());
}

TEST(SymbolTable, staticLocalIsOneDataHome) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), {});
    table.insertFunction("f", functionType.getFunction(), ctx);
    table.startFunction("f", {});
    ASSERT_TRUE(table.insertSymbol("n", type::signedInteger(), ctx, symbols::Storage::Static));
    EXPECT_TRUE(table.hasSymbol("n"));
    EXPECT_EQ(table.lookup("n").getName(), "L$st1_n");
    EXPECT_TRUE(table.lookup("n").isStatic());
    EXPECT_TRUE(table.getCurrentScopeSymbols().empty());

    bool foundHome = false;
    for (const auto& v : table.getDataHomes()) {
        if (v.getName() == "L$st1_n") {
            foundHome = true;
            EXPECT_TRUE(v.isStatic());
        }
    }
    EXPECT_TRUE(foundHome);

    type::Type completed = type::array(type::signedInteger(), 3);
    table.setType("n", completed);
    EXPECT_TRUE(table.lookup("n").getType().isArray());
    EXPECT_EQ(table.lookup("n").getType().getArraySize(), 3);
    for (const auto& v : table.getDataHomes()) {
        if (v.getName() == "L$st1_n") {
            EXPECT_TRUE(v.getType().isArray());
            EXPECT_EQ(v.getType().getArraySize(), 3);
        }
    }

    table.endFunction();
    EXPECT_FALSE(table.hasSymbol("n"));
    foundHome = false;
    for (const auto& v : table.getDataHomes()) {
        if (v.getName() == "L$st1_n") {
            foundHome = true;
            EXPECT_TRUE(v.getType().isArray());
            EXPECT_EQ(v.getType().getArraySize(), 3);
        }
    }
    EXPECT_TRUE(foundHome);
}

TEST(SymbolTable, autoAndStaticCollideInSameBlock) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), {});
    table.insertFunction("f", functionType.getFunction(), ctx);
    table.startFunction("f", {});
    ASSERT_TRUE(table.insertSymbol("n", type::signedInteger(), ctx));
    EXPECT_FALSE(table.insertSymbol("n", type::signedInteger(), ctx, symbols::Storage::Static));
    table.endFunction();
}

TEST(SymbolTable, staticThenAutoCollideInSameBlock) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), {});
    table.insertFunction("f", functionType.getFunction(), ctx);
    table.startFunction("f", {});
    ASSERT_TRUE(table.insertSymbol("n", type::signedInteger(), ctx, symbols::Storage::Static));
    EXPECT_FALSE(table.insertSymbol("n", type::signedInteger(), ctx));
    table.endFunction();
}

TEST(SymbolTable, parameterCollidesWithStaticAndExtern) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), { type::signedInteger() });
    table.insertFunction("f", functionType.getFunction(), ctx);
    table.startFunction("f", { "n" });
    EXPECT_FALSE(table.insertSymbol("n", type::signedInteger(), ctx, symbols::Storage::Static));
    EXPECT_FALSE(table.insertSymbol("n", type::signedInteger(), ctx, symbols::Storage::Extern));
    table.endFunction();
}

TEST(SymbolTable, blockExternTypeMismatchWithFileScope) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), {});
    ASSERT_TRUE(table.insertSymbol("x", type::signedCharacter(), ctx, symbols::Storage::Global));
    table.insertFunction("f", functionType.getFunction(), ctx);
    table.startFunction("f", {});
    EXPECT_FALSE(table.insertSymbol("x", type::signedInteger(), ctx, symbols::Storage::Extern));
    // Failed insert must not leave a function-scope marker that would block a later auto.
    EXPECT_TRUE(table.insertSymbol("x", type::signedInteger(), ctx, symbols::Storage::Automatic));
    table.endFunction();
}

TEST(SymbolTable, blockExternThenAutoCollideInSameBlock) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), {});
    table.insertFunction("f", functionType.getFunction(), ctx);
    table.startFunction("f", {});
    ASSERT_TRUE(table.insertSymbol("x", type::signedInteger(), ctx, symbols::Storage::Extern));
    EXPECT_FALSE(table.insertSymbol("x", type::signedInteger(), ctx, symbols::Storage::Automatic));
    table.endFunction();
}

TEST(SymbolTable, twoStaticsSameBlockCollide) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), {});
    table.insertFunction("f", functionType.getFunction(), ctx);
    table.startFunction("f", {});
    ASSERT_TRUE(table.insertSymbol("n", type::signedInteger(), ctx, symbols::Storage::Static));
    EXPECT_FALSE(table.insertSymbol("n", type::signedInteger(), ctx, symbols::Storage::Static));
    table.endFunction();
}

TEST(SymbolTable, bindFileScopeMergesExternThenDefinition) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    EXPECT_EQ(table.bindFileScopeObject("x", type::signedInteger(), ctx,
                    symbols::Storage::Extern, false),
            ObjectBind::Bound);
    EXPECT_TRUE(table.lookup("x").isExtern());
    EXPECT_FALSE(table.lookup("x").hasDefiningInitializer());
    EXPECT_EQ(table.bindFileScopeObject("x", type::signedInteger(), ctx,
                    symbols::Storage::Global, true),
            ObjectBind::Bound);
    EXPECT_FALSE(table.lookup("x").isExtern());
    EXPECT_TRUE(table.lookup("x").hasDefiningInitializer());
}

TEST(SymbolTable, bindFileScopeRejectsSecondDefiningInitializer) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    EXPECT_EQ(table.bindFileScopeObject("x", type::signedInteger(), ctx,
                    symbols::Storage::Global, true),
            ObjectBind::Bound);
    EXPECT_EQ(table.bindFileScopeObject("x", type::signedInteger(), ctx,
                    symbols::Storage::Global, true),
            ObjectBind::SecondDefinition);
}

TEST(SymbolTable, bindSecondDefinitionDoesNotCompleteArray) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    EXPECT_EQ(table.bindFileScopeObject("a", type::incompleteArray(type::signedInteger()),
                    ctx, symbols::Storage::Global, true),
            ObjectBind::Bound);
    EXPECT_TRUE(table.lookup("a").getType().isIncompleteArray());
    EXPECT_TRUE(table.lookup("a").hasDefiningInitializer());
    EXPECT_EQ(table.bindFileScopeObject("a", type::array(type::signedInteger(), 4),
                    ctx, symbols::Storage::Global, true),
            ObjectBind::SecondDefinition);
    EXPECT_TRUE(table.lookup("a").getType().isIncompleteArray());
}

TEST(SymbolTable, bindFileScopeRejectsQualifierMismatch) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    type::Type plain = type::signedInteger();
    type::Type withConst = type::signedInteger({ type::Qualifier::CONST });
    EXPECT_EQ(table.bindFileScopeObject("x", plain, ctx, symbols::Storage::Extern, false),
            ObjectBind::Bound);
    EXPECT_EQ(table.bindFileScopeObject("x", withConst, ctx, symbols::Storage::Extern, false),
            ObjectBind::TypeConflict);
}

TEST(SymbolTable, bindFileScopeCompletesIncompleteArray) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    EXPECT_EQ(table.bindFileScopeObject("a", type::incompleteArray(type::signedInteger()),
                    ctx, symbols::Storage::Extern, false),
            ObjectBind::Bound);
    EXPECT_EQ(table.bindFileScopeObject("a", type::array(type::signedInteger(), 4),
                    ctx, symbols::Storage::Global, true),
            ObjectBind::Bound);
    EXPECT_EQ(table.lookup("a").getType().getArraySize(), 4);
    EXPECT_TRUE(table.lookup("a").hasDefiningInitializer());
}

TEST(SymbolTable, bindFileScopeKeepsCompleteArraySizeOnIncompleteRedecl) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    EXPECT_EQ(table.bindFileScopeObject("s", type::array(type::signedCharacter(), 6),
                    ctx, symbols::Storage::Global, false),
            ObjectBind::Bound);
    EXPECT_EQ(table.bindFileScopeObject("s", type::incompleteArray(type::signedCharacter()),
                    ctx, symbols::Storage::Extern, false),
            ObjectBind::Bound);
    EXPECT_EQ(table.lookup("s").getType().getArraySize(), 6);
}

TEST(SymbolTable, bindFileScopeStaticThenGlobalIsNonStaticAfterStatic) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    EXPECT_EQ(table.bindFileScopeObject("x", type::signedInteger(), ctx,
                    symbols::Storage::Static, false),
            ObjectBind::Bound);
    EXPECT_EQ(table.bindFileScopeObject("x", type::signedInteger(), ctx,
                    symbols::Storage::Global, false),
            ObjectBind::NonStaticAfterStatic);
}

TEST(SymbolTable, bindFileScopeGlobalThenStaticIsStaticAfterNonStatic) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    EXPECT_EQ(table.bindFileScopeObject("x", type::signedInteger(), ctx,
                    symbols::Storage::Global, false),
            ObjectBind::Bound);
    EXPECT_EQ(table.bindFileScopeObject("x", type::signedInteger(), ctx,
                    symbols::Storage::Static, false),
            ObjectBind::StaticAfterNonStatic);
}

TEST(SymbolTable, bindFileScopeStaticThenExternKeepsInternalLinkage) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    EXPECT_EQ(table.bindFileScopeObject("x", type::signedInteger(), ctx,
                    symbols::Storage::Static, true),
            ObjectBind::Bound);
    EXPECT_EQ(table.bindFileScopeObject("x", type::signedInteger(), ctx,
                    symbols::Storage::Extern, false),
            ObjectBind::Bound);
    EXPECT_TRUE(table.lookup("x").isStatic());
}

} // namespace
