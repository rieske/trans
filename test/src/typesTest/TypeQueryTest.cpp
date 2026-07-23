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
    EXPECT_EQ(codegen::object_abi::valueWords(valueSizeBytes(signedInteger())), 1);
    EXPECT_EQ(codegen::object_abi::valueWords(valueSizeBytes(array(signedInteger(), 3))), 2);
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
