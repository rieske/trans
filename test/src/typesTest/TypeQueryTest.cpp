#include "gtest/gtest.h"

#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"

namespace {

using namespace type;

TEST(TypeQuery, valueIsSignedIntegralsAndDefaults) {
    EXPECT_TRUE(valueIsSigned(signedInteger()));
    EXPECT_FALSE(valueIsSigned(unsignedInteger()));
    EXPECT_TRUE(valueIsSigned(signedCharacter()));
    EXPECT_FALSE(valueIsSigned(unsignedCharacter()));
    // Non-integrals default signed (stack Value / SAR policy).
    EXPECT_TRUE(valueIsSigned(pointer(signedInteger())));
    EXPECT_TRUE(valueIsSigned(array(signedInteger(), 2)));
    EXPECT_TRUE(valueIsSigned(doubleFloating()));
}

TEST(TypeQuery, memoryAccessIsSignedOnlySignedIntegrals) {
    EXPECT_TRUE(memoryAccessIsSigned(signedInteger()));
    EXPECT_FALSE(memoryAccessIsSigned(unsignedInteger()));
    // Loads through pointers / floats zero-extend / don't sign-extend.
    EXPECT_FALSE(memoryAccessIsSigned(pointer(signedInteger())));
    EXPECT_FALSE(memoryAccessIsSigned(doubleFloating()));
}

TEST(TypeQuery, valueSizeBytesAndWords) {
    EXPECT_EQ(valueSizeBytes(signedInteger()), 4);
    EXPECT_EQ(valueSizeBytes(voidType()), 8); // empty → word home
    EXPECT_EQ(type::object_abi::valueWords(valueSizeBytes(signedInteger())), 1);
    EXPECT_EQ(type::object_abi::valueWords(valueSizeBytes(array(signedInteger(), 3))), 2);
}

TEST(TypeQuery, memoryAccessSizeBytes) {
    EXPECT_EQ(memoryAccessSizeBytes(signedCharacter()), 1);
    EXPECT_EQ(memoryAccessSizeBytes(signedShort()), 2);
    EXPECT_EQ(memoryAccessSizeBytes(signedInteger()), 4);
    EXPECT_EQ(memoryAccessSizeBytes(signedLong()), 8);
    EXPECT_EQ(memoryAccessSizeBytes(pointer(signedInteger())), 8);
}

} // namespace

// Predicates must use kind(), not payload presence: pointer-to-T still carries T.
TEST(TypeQuery, isFloatingIgnoresPointerPayload) {
    EXPECT_TRUE(isFloating(doubleFloating()));
    EXPECT_FALSE(isFloating(pointer(doubleFloating())));
    EXPECT_FALSE(isFloating(signedInteger()));
}

TEST(TypeQuery, isIntegralIgnoresPointerPayload) {
    EXPECT_TRUE(isIntegral(signedInteger()));
    EXPECT_FALSE(isIntegral(pointer(signedInteger())));
    EXPECT_FALSE(isIntegral(doubleFloating()));
}

TEST(TypeQuery, isUnsignedSidePointersAndUnsigned) {
    EXPECT_TRUE(isUnsignedSide(pointer(signedInteger())));
    EXPECT_TRUE(isUnsignedSide(array(signedInteger(), 2)));
    EXPECT_TRUE(isUnsignedSide(unsignedInteger()));
    EXPECT_FALSE(isUnsignedSide(signedInteger()));
    EXPECT_FALSE(isUnsignedSide(doubleFloating()));
}

// Pointer-to-float is never floating for codegen/globals; isPrimitive is false too.
TEST(TypeQuery, pointerToFloatIsNotFloatingForCodegenGlobals) {
    type::Type p = type::pointer(type::doubleFloating());
    EXPECT_FALSE(p.isPrimitive());
    EXPECT_FALSE(type::isFloating(p));
    EXPECT_FALSE(type::isIntegral(p));
    EXPECT_TRUE(type::isUnsignedSide(p)); // pointers are address-sized unsigned side
}

TEST(TypeQuery, integralSignednessForShiftsUsesKind) {
    EXPECT_TRUE(type::isIntegral(type::signedInteger()));
    EXPECT_TRUE(type::valueIsSigned(type::signedInteger()));
    EXPECT_FALSE(type::valueIsSigned(type::unsignedInteger()));
    // Pointer must not look like a signed integral for SAR vs SHR policy.
    EXPECT_FALSE(type::isIntegral(type::pointer(type::signedInteger())));
    EXPECT_TRUE(type::valueIsSigned(type::pointer(type::signedInteger()))); // stack Value default
}

// Usual arithmetic result type: promote, wider wins, same-size unsigned over signed.
TEST(TypeQuery, usualArithmeticResultWiderAndUnsignedWins) {
    EXPECT_TRUE(usualArithmeticResult(signedInteger(), doubleFloating()).equivalentTo(doubleFloating()));
    EXPECT_TRUE(usualArithmeticResult(signedCharacter(), signedInteger()).equivalentTo(signedInteger()));
    // same size: unsigned wins over signed
    EXPECT_TRUE(usualArithmeticResult(signedInteger(), unsignedInteger()).equivalentTo(unsignedInteger()));
    EXPECT_TRUE(usualArithmeticResult(unsignedInteger(), signedInteger()).equivalentTo(unsignedInteger()));
    // wider wins even if signed
    EXPECT_TRUE(usualArithmeticResult(unsignedInteger(), signedLong()).equivalentTo(signedLong()));
}

TEST(TypeQuery, needsNumericConvert) {
    EXPECT_TRUE(needsNumericConvert(floating(), signedInteger()));
    EXPECT_TRUE(needsNumericConvert(floating(), doubleFloating()));
    EXPECT_FALSE(needsNumericConvert(floating(), floating()));
    EXPECT_FALSE(needsNumericConvert(signedInteger(), signedLong()));
}

TEST(TypeQuery, usualArithmeticResultKeepsFloat32UntilDouble) {
    auto rf = usualArithmeticResult(floating(), signedInteger());
    EXPECT_TRUE(isFloating(rf));
    EXPECT_EQ(rf.getSize(), 4);
    auto rd = usualArithmeticResult(floating(), doubleFloating());
    EXPECT_TRUE(isFloating(rd));
    EXPECT_EQ(rd.getSize(), 8);
}

TEST(TypeQuery, isProductScalarPrimitivesAndPointers) {
    EXPECT_TRUE(isProductScalar(signedInteger()));
    EXPECT_TRUE(isProductScalar(pointer(signedInteger())));
    EXPECT_FALSE(isProductScalar(array(signedInteger(), 2)));
    EXPECT_FALSE(isProductScalar(voidType()));
    auto st = structure({ { "a", signedInteger() } });
    EXPECT_FALSE(isProductScalar(st));
}

TEST(TypeQuery, productAssignFromIsSoleAssignPolicy) {
    EXPECT_TRUE(productAssignFrom(signedLong(), signedInteger()));
    EXPECT_FALSE(productAssignFrom(signedInteger(), structure({ { "a", signedInteger() } })));
    EXPECT_TRUE(productAssignFrom(pointer(signedInteger()), signedInteger())); // null-ish scalar
}

TEST(TypeQuery, arraySubscriptInfoArrayAndPointerBases) {
    auto arr = array(signedInteger(), 4);
    auto infoA = arraySubscriptInfo(arr);
    EXPECT_TRUE(infoA.baseIsArray);
    EXPECT_TRUE(infoA.elementType.equivalentTo(signedInteger()));
    EXPECT_EQ(infoA.elementStride, 4);

    auto ptr = pointer(signedInteger());
    auto infoP = arraySubscriptInfo(ptr);
    EXPECT_FALSE(infoP.baseIsArray);
    EXPECT_TRUE(infoP.elementType.equivalentTo(signedInteger()));
    EXPECT_EQ(infoP.elementStride, 4);

    auto bad = arraySubscriptInfo(signedInteger());
    EXPECT_EQ(bad.elementStride, 0);
}

// Dual-type bases: multi-dim row keeps expression type T[N] while value is T*.
TEST(TypeQuery, arraySubscriptInfoDualTypeMultiDimRow) {
    auto row = array(signedInteger(), 3); // expression type of a[i] for int a[n][3]
    auto decayed = pointer(signedInteger()); // value type after row decay
    auto info = arraySubscriptInfo(row, decayed);
    EXPECT_FALSE(info.baseIsArray); // base is already a pointer value
    EXPECT_TRUE(info.elementType.equivalentTo(signedInteger()));
    EXPECT_EQ(info.elementStride, 4);
    EXPECT_TRUE(info.valid());
}

// Cast / opaque expression type: fall back to value-type pointer.
TEST(TypeQuery, arraySubscriptInfoDualTypeValuePointerFallback) {
    auto info = arraySubscriptInfo(voidType(), pointer(signedCharacter()));
    EXPECT_FALSE(info.baseIsArray);
    EXPECT_TRUE(info.elementType.equivalentTo(signedCharacter()));
    EXPECT_EQ(info.elementStride, 1);
    EXPECT_TRUE(info.valid());
}

TEST(TypeQuery, arraySubscriptInfoDualTypeInvalid) {
    auto info = arraySubscriptInfo(signedInteger(), signedInteger());
    EXPECT_FALSE(info.valid());
    EXPECT_EQ(info.elementStride, 0);
}

TEST(TypeQuery, productAssignFromComposesValueCompatiblePlusFunctionDestReject) {
    // Assign policy is value compatibility except dest must not be a function type.
    EXPECT_TRUE(productAssignFrom(pointer(signedInteger()), signedInteger()));
    EXPECT_EQ(productAssignFrom(signedInteger(), signedLong()),
            productValueCompatible(signedInteger(), signedLong()));
    auto fn = function(signedInteger(), {});
    // Bare function type as dest is rejected before function->pointer decay.
    EXPECT_FALSE(productAssignFrom(fn, signedInteger()));
    // Pointer-to-function dest remains value-compatible with null-ish scalars.
    EXPECT_TRUE(productAssignFrom(pointer(fn), signedInteger()));
}

TEST(TypeQuery, productArithmeticCompatibleRejectsPointers) {
    auto p = pointer(signedInteger());
    EXPECT_FALSE(productArithmeticCompatible(p, p));
    EXPECT_FALSE(productArithmeticCompatible(p, signedInteger()));
    EXPECT_TRUE(productArithmeticCompatible(signedInteger(), signedLong()));
    EXPECT_TRUE(productArithmeticCompatible(floating(), signedInteger()));
}

TEST(TypeQuery, productValueCompatibleAllowsPointers) {
    auto p = pointer(signedInteger());
    EXPECT_TRUE(productValueCompatible(p, p));
    EXPECT_TRUE(productValueCompatible(p, signedInteger()));
    EXPECT_FALSE(productValueCompatible(signedInteger(), structure({ { "a", signedInteger() } })));
}

