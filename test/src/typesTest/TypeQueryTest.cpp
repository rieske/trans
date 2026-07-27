#include "gtest/gtest.h"

#include "types/Type.h"
#include "types/TypeQuery.h"

namespace {

TEST(TypeQuery, bareAndPointerToFunction) {
    type::Type fn = type::function(type::signedInteger(), {});
    type::Type pfn = type::pointer(fn);
    type::Type ppfn = type::pointer(pfn);

    EXPECT_TRUE(type::isBareFunction(fn));
    EXPECT_FALSE(type::isBareFunction(pfn));
    // Recursive Type: pointer kind is not function (no payload bleed from pointer()).
    EXPECT_FALSE(pfn.isFunction());
    EXPECT_TRUE(type::isPointerToFunction(pfn));
    // pointer-to-pointer-to-function is not pointer-to-function (old bag model lied here).
    EXPECT_FALSE(type::isPointerToFunction(ppfn));
    EXPECT_TRUE(type::isPointerToFunction(ppfn.dereference()));
    EXPECT_TRUE(type::isPointerToBareFunction(pfn));
    EXPECT_FALSE(type::isPointerToBareFunction(ppfn));
}

TEST(TypeQuery, incompleteObjectType) {
    EXPECT_TRUE(type::isIncompleteObjectType(type::voidType()));
    EXPECT_TRUE(type::isIncompleteObjectType(type::function(type::signedInteger(), {})));
    EXPECT_TRUE(type::isIncompleteObjectType(type::incompleteStructure()));
    EXPECT_FALSE(type::isIncompleteObjectType(type::signedInteger()));
    EXPECT_FALSE(type::isIncompleteObjectType(type::pointer(type::voidType())));
}

TEST(TypeQuery, productCanAssignScalarsAndPointers) {
    type::Type i = type::signedInteger();
    type::Type pi = type::pointer(i);
    EXPECT_TRUE(type::productCanAssignFrom(i, i));
    EXPECT_TRUE(type::productCanAssignFrom(pi, pi));
    EXPECT_TRUE(type::productCanAssignFrom(pi, i)); // null constant int → pointer
    EXPECT_TRUE(type::productCanAssignFrom(i, pi)); // product-loose scalar
}

TEST(TypeQuery, productCanAssignFunctionPointer) {
    type::Type fn = type::function(type::signedInteger(), {});
    type::Type pfn = type::pointer(fn);
    type::Type i = type::signedInteger();
    EXPECT_TRUE(type::productCanAssignFrom(pfn, fn));
    EXPECT_TRUE(type::productCanAssignFrom(pfn, pfn));
    EXPECT_TRUE(type::productCanAssignFrom(pfn, i)); // null
    EXPECT_FALSE(type::productCanAssignFrom(i, fn));
    EXPECT_FALSE(type::productCanAssignFrom(i, pfn));
    std::string msg = type::productAssignFailureMessage(i, fn);
    EXPECT_NE(msg.find("function"), std::string::npos);
}

TEST(TypeQuery, productCanAssignStructures) {
    auto s = type::structure({ { "x", type::signedInteger() } });
    auto t = type::structure({ { "y", type::signedInteger() } });
    EXPECT_TRUE(type::productCanAssignFrom(s, s));
    EXPECT_TRUE(type::productCanAssignFrom(s, t)); // product-loose structure-to-structure
    EXPECT_FALSE(type::productCanAssignFrom(s, type::signedInteger()));
    EXPECT_FALSE(type::productCanAssignFrom(type::signedInteger(), s));
}

TEST(TypeQuery, productRejectsArrayAndVoidAndIncomplete) {
    type::Type arr = type::array(type::signedInteger(), 3);
    type::Type i = type::signedInteger();
    EXPECT_FALSE(type::productCanAssignFrom(arr, i));
    EXPECT_FALSE(type::productCanAssignFrom(i, arr));
    EXPECT_FALSE(type::productCanAssignFrom(type::voidType(), i));
    EXPECT_FALSE(type::productCanAssignFrom(type::incompleteStructure(), i));
}

TEST(TypeQuery, arraySubscriptInfoArrayAndPointer) {
    type::Type arr = type::array(type::signedInteger(), 4);
    auto info = type::arraySubscriptInfo(arr);
    EXPECT_TRUE(info.valid());
    EXPECT_TRUE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.isPrimitive());

    type::Type ptr = type::pointer(type::signedInteger());
    auto pinfo = type::arraySubscriptInfo(ptr);
    EXPECT_TRUE(pinfo.valid());
    EXPECT_FALSE(pinfo.baseIsArray);
}

TEST(TypeQuery, arraySubscriptInfoDualTypeRow) {
    type::Type row = type::array(type::signedInteger(), 3);
    type::Type decayed = type::pointer(type::signedInteger());
    auto info = type::arraySubscriptInfo(row, decayed);
    EXPECT_TRUE(info.valid());
    EXPECT_FALSE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.isPrimitive());
}

TEST(TypeQuery, arraySubscriptInfoInvalidBase) {
    auto info = type::arraySubscriptInfo(type::signedInteger());
    EXPECT_FALSE(info.valid());
}


TEST(TypeQuery, classifyPointerArithmeticForms) {
    type::Type i = type::signedInteger();
    type::Type pi = type::pointer(i);

    auto none = type::classifyPointerArithmetic(i, i, '+');
    EXPECT_EQ(none.form, type::PointerArithmeticForm::None);

    auto ppi = type::classifyPointerArithmetic(pi, i, '+');
    EXPECT_EQ(ppi.form, type::PointerArithmeticForm::PtrPlusInt);
    EXPECT_TRUE(ppi.resultType.isPointer());
    EXPECT_EQ(ppi.strideBytes, 4);

    auto ipp = type::classifyPointerArithmetic(i, pi, '+');
    EXPECT_EQ(ipp.form, type::PointerArithmeticForm::IntPlusPtr);

    auto pmi = type::classifyPointerArithmetic(pi, i, '-');
    EXPECT_EQ(pmi.form, type::PointerArithmeticForm::PtrMinusInt);

    auto pmp = type::classifyPointerArithmetic(pi, pi, '-');
    EXPECT_EQ(pmp.form, type::PointerArithmeticForm::PtrMinusPtr);
    EXPECT_TRUE(pmp.resultType.isPrimitive());

    auto inv = type::classifyPointerArithmetic(pi, pi, '+');
    EXPECT_EQ(inv.form, type::PointerArithmeticForm::Invalid);
}

TEST(TypeQuery, arraySubscriptInfoDualFallbackToValuePointer) {
    // Non-array/non-pointer expression type, pointer value type.
    type::Type expr = type::signedInteger();
    type::Type val = type::pointer(type::signedInteger());
    auto info = type::arraySubscriptInfo(expr, val);
    EXPECT_TRUE(info.valid());
    EXPECT_FALSE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.isPrimitive());
}

TEST(TypeQuery, arraySubscriptInfoDualFallsThroughToExpr) {
    type::Type expr = type::pointer(type::signedInteger());
    type::Type val = type::signedInteger();
    auto info = type::arraySubscriptInfo(expr, val);
    EXPECT_TRUE(info.valid());
    EXPECT_FALSE(info.baseIsArray);
}

TEST(TypeQuery, incompleteMemberOrElement) {
    EXPECT_TRUE(type::isIncompleteMemberOrElementType(type::voidType()));
    EXPECT_TRUE(type::isIncompleteMemberOrElementType(type::function(type::signedInteger(), {})));
    EXPECT_TRUE(type::isIncompleteMemberOrElementType(type::incompleteStructure()));
    EXPECT_FALSE(type::isIncompleteMemberOrElementType(type::pointer(type::voidType())));
}

TEST(TypeQuery, productAssignFailureMessageTypeMismatch) {
    std::string msg = type::productAssignFailureMessage(type::signedInteger(),
            type::structure({ { "x", type::signedInteger() } }));
    EXPECT_NE(msg.find("type mismatch"), std::string::npos);
}

TEST(TypeQuery, productAssignFromMatchesCanAssignAlias) {
    type::Type i = type::signedInteger();
    type::Type pi = type::pointer(i);
    EXPECT_EQ(type::productAssignFrom(i, i), type::productCanAssignFrom(i, i));
    EXPECT_EQ(type::productAssignFrom(pi, i), type::productCanAssignFrom(pi, i));
    EXPECT_EQ(type::productAssignFrom(i, pi), type::productCanAssignFrom(i, pi));
}

TEST(TypeQuery, productValueCompatibleScalarsAndPointers) {
    type::Type i = type::signedInteger();
    type::Type u = type::unsignedInteger();
    type::Type pi = type::pointer(i);
    EXPECT_TRUE(type::productValueCompatible(i, u));
    EXPECT_TRUE(type::productValueCompatible(pi, pi));
    EXPECT_TRUE(type::productValueCompatible(i, pi)); // decay not required for product-loose
    EXPECT_FALSE(type::productValueCompatible(i, type::voidType()));
}

TEST(TypeQuery, productArithmeticCompatible) {
    EXPECT_TRUE(type::productArithmeticCompatible(type::signedInteger(), type::signedLong()));
    EXPECT_TRUE(type::productArithmeticCompatible(type::floating(), type::doubleFloating()));
    EXPECT_FALSE(type::productArithmeticCompatible(type::pointer(type::signedInteger()), type::signedInteger()));
    EXPECT_FALSE(type::productArithmeticCompatible(type::signedInteger(), type::voidType()));
}

TEST(TypeQuery, isFloatingAndIntegral) {
    EXPECT_TRUE(type::isIntegral(type::signedInteger()));
    EXPECT_TRUE(type::isFloating(type::floating()));
    EXPECT_FALSE(type::isFloating(type::signedInteger()));
    EXPECT_FALSE(type::isIntegral(type::floating()));
}

TEST(TypeQuery, usualArithmeticResult) {
    auto r = type::usualArithmeticResult(type::signedCharacter(), type::signedInteger());
    EXPECT_TRUE(r.isPrimitive());
    EXPECT_EQ(r.getSize(), 4);
    auto rf = type::usualArithmeticResult(type::floating(), type::signedInteger());
    EXPECT_TRUE(type::isFloating(rf));
    EXPECT_EQ(rf.getSize(), 8); // product promotes to double
}

TEST(TypeQuery, arraySubscriptPointerToArrayStride) {
    // p is int (*)[3]: p[i] steps by sizeof(int[3])
    type::Type row = type::array(type::signedInteger(), 3);
    type::Type p = type::pointer(row);
    auto info = type::arraySubscriptInfo(p);
    EXPECT_TRUE(info.valid());
    EXPECT_FALSE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.isArray());
    EXPECT_EQ(info.elementStride, 12);
}
} // namespace
