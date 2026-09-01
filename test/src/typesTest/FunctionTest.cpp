#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Type.h"

namespace {

using namespace testing;

TEST(Function, copyAssignment) {
    type::Function t = type::function(type::voidType(), {type::signedInteger()}).getFunction();
    EXPECT_THAT(t.to_string(), Eq("void(int)"));

    type::Function t2 = type::function(type::signedInteger()).getFunction();
    EXPECT_THAT(t2.to_string(), Eq("int()"));

    t2 = t;
    EXPECT_THAT(t2.to_string(), Eq("void(int)"));
}

TEST(Function, typeCopyStaysEquivalentAndIndependent) {
    auto original = type::function(
            type::voidType(), { type::signedInteger(), type::pointer(type::signedCharacter()) }, true);
    auto copy = original;
    EXPECT_TRUE(copy.equivalentTo(original));
    EXPECT_EQ(copy.to_string(), original.to_string());

    copy = type::function(type::signedInteger());
    EXPECT_TRUE(original.isFunction());
    EXPECT_TRUE(original.equivalentTo(type::function(
            type::voidType(), { type::signedInteger(), type::pointer(type::signedCharacter()) }, true)));
    EXPECT_TRUE(copy.equivalentTo(type::function(type::signedInteger())));
}

TEST(Function, accessorsShareUntilAssigned) {
    auto fn = type::function(type::voidType(), { type::signedInteger() }).getFunction();
    auto copy = fn;
    EXPECT_EQ(&fn.getReturnType(), &copy.getReturnType());
    EXPECT_EQ(&fn.getArguments(), &copy.getArguments());
    EXPECT_EQ(fn.getArguments().size(), 1u);
    EXPECT_TRUE(fn.getArguments().front().equivalentTo(type::signedInteger()));

    copy = type::function(type::signedInteger()).getFunction();
    EXPECT_TRUE(fn.getReturnType().isVoid());
    EXPECT_TRUE(copy.getReturnType().isPrimitive());
    EXPECT_TRUE(copy.getArguments().empty());
}

} // namespace

