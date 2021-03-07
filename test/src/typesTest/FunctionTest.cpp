#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Type.h"

namespace {

using namespace testing;

TEST(Function, copyAssignment) {
    auto t = type::function(type::voidType(), {type::signedInteger()}).getFunction();

    EXPECT_THAT(t.to_string(), Eq("void(int)"));

    type::Function t2 = t;

    EXPECT_THAT(t2.to_string(), Eq("void(int)"));
}

} // namespace

