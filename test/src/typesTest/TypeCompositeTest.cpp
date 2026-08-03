#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Type.h"

namespace {

using namespace testing;
using namespace type;

TEST(Type, compatibleWithIncompleteAndCompleteArray) {
    auto i = signedInteger();
    auto inc = incompleteArray(i);
    auto three = array(i, 3);
    auto two = array(i, 2);
    EXPECT_THAT(inc.compatibleWith(three), IsTrue());
    EXPECT_THAT(three.compatibleWith(inc), IsTrue());
    EXPECT_THAT(inc.compatibleWith(inc), IsTrue());
    EXPECT_THAT(three.compatibleWith(array(i, 3)), IsTrue());
    EXPECT_THAT(three.compatibleWith(two), IsFalse());
    EXPECT_THAT(inc.sameQualifiedType(three), IsFalse());

    auto ci = signedInteger({ Qualifier::CONST });
    EXPECT_THAT(incompleteArray(i).compatibleWith(array(ci, 1)), IsFalse());
    EXPECT_THAT(incompleteArray(ci).compatibleWith(array(ci, 1)), IsTrue());
}

TEST(Type, compatibleWithNestedArrayAndPointerToArray) {
    auto i = signedInteger();
    auto incRows = incompleteArray(array(i, 3));
    auto twoByThree = array(array(i, 3), 2);
    auto twoByFour = array(array(i, 4), 2);
    EXPECT_THAT(incRows.compatibleWith(twoByThree), IsTrue());
    EXPECT_THAT(incRows.compatibleWith(twoByFour), IsFalse());

    auto pInc = pointer(incompleteArray(i));
    auto pFour = pointer(array(i, 4));
    EXPECT_THAT(pInc.compatibleWith(pFour), IsTrue());
    EXPECT_THAT(pInc.compatibleWith(pointer(array(i, 3))), IsTrue());
    EXPECT_THAT(pointer(array(i, 4)).compatibleWith(pointer(array(i, 3))), IsFalse());
}

TEST(Type, compositePrefersCompleteArrayBound) {
    auto i = signedInteger();
    auto inc = incompleteArray(i);
    auto three = array(i, 3);

    auto fromInc = inc.composite(three);
    ASSERT_TRUE(fromInc.has_value());
    EXPECT_THAT(fromInc->sameQualifiedType(three), IsTrue());
    EXPECT_THAT(fromInc->getArraySize(), Eq(3));

    auto fromComplete = three.composite(inc);
    ASSERT_TRUE(fromComplete.has_value());
    EXPECT_THAT(fromComplete->sameQualifiedType(three), IsTrue());

    EXPECT_FALSE(three.composite(array(i, 2)).has_value());

    auto bothInc = inc.composite(incompleteArray(i));
    ASSERT_TRUE(bothInc.has_value());
    EXPECT_THAT(bothInc->isIncompleteArray(), IsTrue());
}

TEST(Type, compositeNestedArrayAndPointerToArray) {
    auto i = signedInteger();
    auto twoByThree = array(array(i, 3), 2);
    auto merged = incompleteArray(array(i, 3)).composite(twoByThree);
    ASSERT_TRUE(merged.has_value());
    EXPECT_THAT(merged->sameQualifiedType(twoByThree), IsTrue());

    auto pFour = pointer(array(i, 4));
    auto pMerged = pointer(incompleteArray(i)).composite(pFour);
    ASSERT_TRUE(pMerged.has_value());
    EXPECT_THAT(pMerged->sameQualifiedType(pFour), IsTrue());
}

} // namespace
