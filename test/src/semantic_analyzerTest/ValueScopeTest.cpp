#include "gtest/gtest.h"

#include "semantic_analyzer/ValueScope.h"
#include "types/Type.h"

namespace {

using namespace semantic_analyzer;

translation_unit::Context ctx() {
    return translation_unit::Context { "test", 1 };
}

// ValueScope is insert-once. File-scope merge lives on SymbolTable::bindFileScopeObject.
TEST(ValueScope, insertOnceRejectsSameName) {
    ValueScope scope;
    type::Type i = type::signedInteger();
    ASSERT_TRUE(scope.insertSymbol("x", i, ctx(), symbols::Storage::Global, "x"));
    EXPECT_FALSE(scope.insertSymbol("x", i, ctx(), symbols::Storage::Global, "x"));
}

TEST(ValueScope, localRedeclAlwaysRejected) {
    ValueScope scope;
    ASSERT_TRUE(scope.insertSymbol("x", type::signedInteger(), ctx(),
            symbols::Storage::Automatic, "x"));
    EXPECT_FALSE(scope.insertSymbol("x", type::signedInteger(), ctx(),
            symbols::Storage::Automatic, "x"));
}

TEST(ValueScope, promoteExternToDefinition) {
    ValueScope scope;
    ASSERT_TRUE(scope.insertSymbol("x", type::signedInteger(), ctx(), symbols::Storage::Extern, "x"));
    EXPECT_TRUE(scope.lookup("x").isExtern());
    scope.promoteExternToDefinition("x");
    EXPECT_FALSE(scope.lookup("x").isExtern());
    EXPECT_TRUE(scope.lookup("x").isGlobal());
}

TEST(ValueScope, markDefiningInitializer) {
    ValueScope scope;
    ASSERT_TRUE(scope.insertSymbol("x", type::signedInteger(), ctx(), symbols::Storage::Global, "x"));
    EXPECT_FALSE(scope.lookup("x").hasDefiningInitializer());
    scope.markDefiningInitializer("x");
    EXPECT_TRUE(scope.lookup("x").hasDefiningInitializer());
}

} // namespace
