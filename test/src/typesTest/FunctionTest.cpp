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

} // namespace

