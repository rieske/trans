#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Type.h"
#include "types/TypeQuery.h"

namespace {

using namespace testing;

TEST(Type, sameQualifiedTypeRespectsQualifiersAtEachLevel) {
    using namespace type;
    auto i = signedInteger();
    auto ci = signedInteger({ Qualifier::CONST });
    EXPECT_THAT(i.sameQualifiedType(i), IsTrue());
    EXPECT_THAT(i.sameQualifiedType(ci), IsFalse());
    EXPECT_THAT(ci.sameQualifiedType(i), IsFalse());
    EXPECT_THAT(pointer(i).sameQualifiedType(pointer(i)), IsTrue());
    EXPECT_THAT(pointer(i).sameQualifiedType(pointer(ci)), IsFalse());
    EXPECT_THAT(pointer(ci).sameQualifiedType(pointer(i)), IsFalse());
    EXPECT_THAT(pointer(i).equivalentTo(pointer(ci)), IsTrue());
    EXPECT_THAT(incompleteArray(i).sameQualifiedType(array(i, 3)), IsFalse());
}

TEST(Type, productAssignFromCompatiblePrimitives) {
    auto i = type::signedInteger();
    auto l = type::signedLong();
    auto f = type::floating();
    EXPECT_TRUE(type::productAssignFrom(i, i));
    EXPECT_TRUE(type::productAssignFrom(l, l));
    // Product-loose: any primitive accepts any primitive.
    EXPECT_TRUE(type::productAssignFrom(i, l));
    EXPECT_TRUE(type::productAssignFrom(l, i));
    EXPECT_TRUE(type::productAssignFrom(i, f));
    EXPECT_TRUE(type::productAssignFrom(f, i));
}

TEST(Type, productAssignFromPointersAndNullConstant) {
    auto pi = type::pointer(type::signedInteger());
    auto pc = type::pointer(type::signedCharacter());
    auto i = type::signedInteger();
    EXPECT_TRUE(type::productAssignFrom(pi, pi));
    EXPECT_TRUE(type::productAssignFrom(pi, pc));
    // Integer → pointer (null pointer constant / product scalar mix).
    EXPECT_TRUE(type::productAssignFrom(pi, i));
    // Array decays to pointer for the source.
    auto arr = type::array(type::signedInteger(), 4);
    EXPECT_TRUE(type::productAssignFrom(pi, arr));
}

TEST(Type, productAssignFromRejectsStructScalarMismatch) {
    auto st = type::structure({ { "a", type::signedInteger() } });
    auto i = type::signedInteger();
    EXPECT_FALSE(type::productAssignFrom(i, st));
    EXPECT_FALSE(type::productAssignFrom(st, i));
    EXPECT_FALSE(type::productAssignFrom(type::voidType(), i));
    EXPECT_FALSE(type::productAssignFrom(i, type::voidType()));
}

TEST(Type, productAssignFromRejectsDestArrayAndDecaysSourceArray) {
    auto arr = type::array(type::signedCharacter(), 4);
    auto p = type::pointer(type::signedCharacter());
    auto i = type::signedInteger();
    EXPECT_FALSE(type::productAssignFrom(arr, p));
    EXPECT_FALSE(type::productAssignFrom(arr, i));
    EXPECT_FALSE(type::productAssignFrom(arr, arr));
    EXPECT_TRUE(type::productAssignFrom(p, arr));
}

TEST(Type, productAssignFromPointerToStructUnionForSockaddrArg) {
    // glibc __CONST_SOCKADDR_ARG: transparent union of sockaddr pointer members.
    // Callers pass struct sockaddr *; the parameter type is a union.
    auto sockaddr = type::structure({ { "sa_family", type::signedShort() } });
    auto sockaddrPtr = type::pointer(sockaddr);
    auto sockaddrArgUnion = type::unionType({
            { "__sockaddr__", sockaddrPtr },
    });
    EXPECT_TRUE(type::productAssignFrom(sockaddrArgUnion, sockaddrPtr));
    EXPECT_TRUE(type::productAssignFrom(sockaddrPtr, sockaddrArgUnion));
    // Still reject bare scalar ↔ struct.
    EXPECT_FALSE(type::productAssignFrom(sockaddr, type::signedInteger()));
}

TEST(Type, productAssignFromSameStructBodyOnly) {
    auto st1 = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
    });
    auto st2 = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
    });
    EXPECT_TRUE(type::productAssignFrom(st1, st1));
    // Distinct structure() calls allocate distinct bodies even with same layout.
    EXPECT_FALSE(type::productAssignFrom(st1, st2));
}

TEST(Type, equivalentToIgnoresTopLevelQualifiersAndComparesStructure) {
    auto i = type::signedInteger();
    auto ci = type::signedInteger({ type::Qualifier::CONST });
    EXPECT_TRUE(i.equivalentTo(ci));
    EXPECT_TRUE(type::pointer(i).equivalentTo(type::pointer(ci)));
    EXPECT_FALSE(type::signedInteger().equivalentTo(type::unsignedInteger()));
    EXPECT_FALSE(type::array(i, 3).equivalentTo(type::array(i, 4)));
    EXPECT_TRUE(type::array(i, 3).equivalentTo(type::array(i, 3)));

    auto st1 = type::structure({ { "a", type::signedLong() } });
    auto st2 = type::structure({ { "a", type::signedLong() } });
    EXPECT_TRUE(st1.equivalentTo(st1));
    EXPECT_FALSE(st1.equivalentTo(st2));

    auto f1 = type::function(type::signedInteger(), { type::signedInteger() }, false);
    auto f2 = type::function(type::signedInteger(), { type::signedInteger({ type::Qualifier::CONST }) }, false);
    auto f3 = type::function(type::signedInteger(), { type::signedInteger() }, true);
    EXPECT_TRUE(f1.equivalentTo(f2));
    EXPECT_FALSE(f1.equivalentTo(f3));
}

TEST(Type, productAssignFromRejectsPointerIntoNonTransparentStruct) {
    // Not a transparent union of pointers — must not use the sockaddr allowance.
    auto st = type::structure({ { "a", type::signedInteger() } });
    auto p = type::pointer(type::signedInteger());
    EXPECT_FALSE(type::productAssignFrom(st, p));
    EXPECT_FALSE(type::productAssignFrom(p, st));
}

TEST(Type, productValueCompatibleRecordsIgnoreBodyIdentity) {
    // Value-compat is intentionally looser than assign (see TypeQuery.h).
    auto st1 = type::structure({ { "a", type::signedLong() } });
    auto st2 = type::structure({ { "a", type::signedLong() } });
    EXPECT_TRUE(type::productValueCompatible(st1, st2));
    EXPECT_FALSE(type::productAssignFrom(st1, st2));
}

TEST(Type, productLoosePointerAssignIsPermanent) {
    // Permanent product rule: typed pointer mixing without ISO conversion ranks.
    auto pi = type::pointer(type::signedInteger());
    auto pc = type::pointer(type::signedCharacter());
    auto pv = type::pointer(type::voidType());
    EXPECT_TRUE(type::productAssignFrom(pi, pc));
    EXPECT_TRUE(type::productAssignFrom(pc, pi));
    EXPECT_TRUE(type::productAssignFrom(pv, pi));
    EXPECT_TRUE(type::productAssignFrom(pi, pv));
}

TEST(Type, productFunctionDesignatorOnlyToFunctionPointer) {
    auto fn = type::function(type::signedInteger(), { type::signedInteger() });
    auto pfn = type::pointer(fn);
    auto pvoid = type::pointer(type::voidType());
    auto pint = type::pointer(type::signedInteger());
    // Bare function as source (designator before decay).
    EXPECT_TRUE(type::productAssignFrom(pfn, fn));
    EXPECT_FALSE(type::productAssignFrom(pvoid, fn));
    EXPECT_FALSE(type::productAssignFrom(pint, fn));
    EXPECT_FALSE(type::productAssignFrom(type::signedInteger(), fn));
}

} // namespace
