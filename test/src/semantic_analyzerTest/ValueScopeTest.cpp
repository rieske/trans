#include "gtest/gtest.h"

#include "semantic_analyzer/ValueScope.h"
#include "types/Type.h"

namespace {

using namespace semantic_analyzer;

translation_unit::Context ctx() {
    return translation_unit::Context { "test", 1 };
}

// File-scope redecl: structural equality via equivalentTo (not to_string formatting).
TEST(ValueScope, fileScopeRedeclAcceptsEquivalentTypes) {
    ValueScope scope;
    type::Type i = type::signedInteger();
    ASSERT_TRUE(scope.insertSymbol("x", i, ctx(), /*global=*/true));
    // Same type again is compatible redecl.
    EXPECT_TRUE(scope.insertSymbol("x", i, ctx(), /*global=*/true));
}

TEST(ValueScope, fileScopeRedeclAcceptsTopLevelQualifierDifferenceViaEquivalentTo) {
    ValueScope scope;
    type::Type plain = type::signedInteger();
    type::Type withConst = type::signedInteger({ type::Qualifier::CONST });
    ASSERT_TRUE(scope.insertSymbol("c", plain, ctx(), true));
    // equivalentTo ignores top-level const/volatile.
    EXPECT_TRUE(plain.equivalentTo(withConst));
    EXPECT_TRUE(scope.insertSymbol("c", withConst, ctx(), true));
}

TEST(ValueScope, fileScopeIncompleteArrayCompletedByDefinition) {
    ValueScope scope;
    type::Type incomplete = type::array(type::signedInteger(), 0);
    type::Type complete = type::array(type::signedInteger(), 4);
    ASSERT_TRUE(scope.insertSymbol("a", incomplete, ctx(), true));
    EXPECT_TRUE(scope.insertSymbol("a", complete, ctx(), true));
    EXPECT_EQ(scope.lookup("a").getType().getArraySize(), 4);
}

TEST(ValueScope, fileScopeCompleteThenIncompleteArrayKeepsPriorSize) {
    ValueScope scope;
    type::Type complete = type::array(type::signedCharacter(), 6);
    type::Type incomplete = type::array(type::signedCharacter(), 0);
    ASSERT_TRUE(scope.insertSymbol("s", complete, ctx(), true));
    EXPECT_TRUE(scope.insertSymbol("s", incomplete, ctx(), true));
    EXPECT_EQ(scope.lookup("s").getType().getArraySize(), 6);
}

TEST(ValueScope, fileScopeRejectsIncompatibleTypes) {
    ValueScope scope;
    ASSERT_TRUE(scope.insertSymbol("n", type::signedInteger(), ctx(), true));
    EXPECT_FALSE(scope.insertSymbol("n", type::signedLong(), ctx(), true));
}

TEST(ValueScope, localRedeclAlwaysRejected) {
    ValueScope scope;
    ASSERT_TRUE(scope.insertSymbol("x", type::signedInteger(), ctx(), false));
    EXPECT_FALSE(scope.insertSymbol("x", type::signedInteger(), ctx(), false));
}

} // namespace
