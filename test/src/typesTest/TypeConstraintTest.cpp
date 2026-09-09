#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Type.h"
#include "types/TypeConstraint.h"

#include <stdexcept>

namespace {

using namespace testing;
using namespace type;

TEST(TypeConstraint, variableArrayOfIncompleteElement) {
    EXPECT_STREQ(arrayTypeError(variableArray(incompleteArray(signedInteger()))),
            "array of incomplete type");
    EXPECT_STREQ(arrayTypeError(variableArray(voidType())), "array of incomplete type");
}

TEST(TypeConstraint, arrayOfIncompleteArray) {
    auto a = array(incompleteArray(signedInteger()), 2);
    EXPECT_TRUE(a.isArray());
    EXPECT_STREQ(arrayTypeError(a), "array of incomplete type");
}

TEST(TypeConstraint, arrayOfBareFunction) {
    auto a = array(function(signedInteger(), {}), 2);
    EXPECT_TRUE(a.isArray());
    EXPECT_STREQ(arrayTypeError(a), "array of incomplete type");
}

TEST(TypeConstraint, arrayNegativeCountIsLogicError) {
    EXPECT_THROW(array(signedInteger(), -1), std::logic_error);
}

TEST(TypeConstraint, arraySizeOverflow) {
    auto a = array(signedInteger(), 536870913);
    EXPECT_THAT(a.getSize(), Eq(0));
    EXPECT_STREQ(arrayTypeError(a), "array size is too large");
}

TEST(TypeConstraint, arrayOfVoid) {
    auto a = array(voidType(), 3);
    EXPECT_TRUE(a.isArray());
    EXPECT_STREQ(arrayTypeError(a), "array of incomplete type");
}

TEST(TypeConstraint, arrayOfIncompleteRecord) {
    auto a = array(incompleteRecord(), 3);
    EXPECT_TRUE(a.isArray());
    EXPECT_STREQ(arrayTypeError(a), "array of incomplete type");
}

TEST(TypeConstraint, arrayTypeErrorSeesFunctionReturnNotAdjustedParams) {
    EXPECT_STREQ(arrayTypeError(function(array(voidType(), 3))), "array of incomplete type");
    EXPECT_STREQ(arrayTypeError(function(pointer(array(voidType(), 3)))),
            "array of incomplete type");
    auto adjusted = function(signedInteger(), { pointer(voidType()) });
    EXPECT_EQ(arrayTypeError(adjusted), nullptr);
}

TEST(TypeConstraint, bitFieldRejectsIllegalWidthAndType) {
    auto badWide = incompleteRecord();
    EXPECT_STREQ(completeStructure(badWide, { MemberSpec { "x", signedInteger(), 33 } }),
            "width of bit-field exceeds its type");
    EXPECT_TRUE(badWide.isIncompleteRecord());
    auto badZero = incompleteRecord();
    EXPECT_STREQ(completeStructure(badZero, { MemberSpec { "x", signedInteger(), 0 } }),
            "zero width for bit-field");
    EXPECT_TRUE(badZero.isIncompleteRecord());
    auto badType = incompleteRecord();
    EXPECT_STREQ(completeStructure(badType, { MemberSpec { "f", floating(), 3 } }),
            "bit-field has non-integer type");
    EXPECT_TRUE(badType.isIncompleteRecord());
    auto badInt128 = incompleteRecord();
    EXPECT_STREQ(completeStructure(badInt128, { MemberSpec { "x", signedInt128(), 8 } }),
            "bit-field type is too wide");
    EXPECT_TRUE(badInt128.isIncompleteRecord());
    auto badInt128Width = incompleteRecord();
    EXPECT_STREQ(completeStructure(badInt128Width, { MemberSpec { "x", signedInt128(), 80 } }),
            "bit-field type is too wide");
    EXPECT_TRUE(badInt128Width.isIncompleteRecord());
}

TEST(TypeConstraint, structureRejectsIncompleteMembers) {
    auto voidMember = structure({{"v", voidType()}});
    EXPECT_TRUE(voidMember.isIncompleteRecord());
    EXPECT_THAT(voidMember.memberCount(), Eq(0));
    auto fnMember = structure({{"f", function(signedInteger(), {})}});
    EXPECT_TRUE(fnMember.isIncompleteRecord());
    EXPECT_THAT(fnMember.memberCount(), Eq(0));
}

TEST(TypeConstraint, structureRejectsFlexibleArrayIfNotLast) {
    auto s = structure({
            { "data", incompleteArray(signedInteger()) },
            { "n", signedInteger() },
    });
    EXPECT_TRUE(s.isIncompleteRecord());
    EXPECT_THAT(s.memberCount(), Eq(0));
}

TEST(TypeConstraint, structureRejectsFlexibleArrayAsOnlyMember) {
    auto s = structure({
            { "data", incompleteArray(signedInteger()) },
    });
    EXPECT_TRUE(s.isIncompleteRecord());
    EXPECT_THAT(s.memberCount(), Eq(0));
}

TEST(TypeConstraint, unionRejectsFlexibleArrayMember) {
    auto u = unionType({
            { "n", signedInteger() },
            { "data", incompleteArray(signedInteger()) },
    });
    EXPECT_TRUE(u.isIncompleteRecord());
    EXPECT_THAT(u.memberCount(), Eq(0));
}

TEST(TypeConstraint, structureRejectsDuplicateMemberNames) {
    auto s = structure({{"x", signedInteger()}, {"x", signedCharacter()}});
    EXPECT_TRUE(s.isIncompleteRecord());
    EXPECT_THAT(s.memberCount(), Eq(0));
}

TEST(TypeConstraint, structureRejectsSizeOverflow) {
    auto huge = array(signedCharacter(), 1073741824);
    auto s = structure({{"a", huge}, {"b", huge}});
    EXPECT_TRUE(s.isIncompleteRecord());
    EXPECT_THAT(s.memberCount(), Eq(0));
}

TEST(TypeConstraint, completeStructureFailurePreservesPriorSharedLayout) {
    auto tag = incompleteRecord();
    completeStructure(tag, { MemberSpec { "x", signedInteger() } });
    EXPECT_THAT(tag.getSize(), Eq(4));
    auto alias = tag;
    auto ptr = pointer(tag);

    EXPECT_STREQ(completeStructure(tag, { MemberSpec { "y", incompleteRecord() } }),
            "structure member has incomplete type");

    EXPECT_THAT(tag.getSize(), Eq(4));
    EXPECT_THAT(alias.getSize(), Eq(4));
    EXPECT_THAT(ptr.dereference().getSize(), Eq(4));
    auto found = lookupMember(tag, "x");
    ASSERT_TRUE(found);
    EXPECT_THAT(found->offsetBytes, Eq(0));
    EXPECT_THAT(tag.isCompleteRecord(), IsTrue());
}

} // namespace
