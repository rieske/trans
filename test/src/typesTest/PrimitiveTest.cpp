#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Type.h"

namespace {

using namespace testing;

TEST(Primitive, signedCharacter) {
    auto t = type::signedCharacter().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(1));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isCharacter(), IsTrue());
    EXPECT_THAT(t.isBoolean(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("char"));
}

TEST(Primitive, unsignedCharacter) {
    auto t = type::unsignedCharacter().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(1));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isCharacter(), IsTrue());
    EXPECT_THAT(t.isBoolean(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("unsigned char"));
}

TEST(Primitive, booleanIsDistinctFromUnsignedCharacter) {
    auto b = type::boolean().getPrimitive();
    auto c = type::unsignedCharacter().getPrimitive();

    EXPECT_THAT(b.getSize(), Eq(1));
    EXPECT_THAT(b.isSigned(), IsFalse());
    EXPECT_THAT(b.isFloating(), IsFalse());
    EXPECT_THAT(b.isBoolean(), IsTrue());
    EXPECT_THAT(b.isCharacter(), IsFalse());
    EXPECT_THAT(b.kind(), Eq(type::PrimitiveKind::Boolean));
    EXPECT_THAT(c.kind(), Eq(type::PrimitiveKind::UnsignedChar));
    EXPECT_THAT(b.to_string(), Eq("bool"));
    EXPECT_THAT(c.isBoolean(), IsFalse());
    EXPECT_THAT(b.equivalentTo(c), IsFalse());
    EXPECT_THAT(b.equivalentTo(type::boolean().getPrimitive()), IsTrue());
}

TEST(Primitive, signedInteger) {
    auto t = type::signedInteger().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("int"));
}

TEST(Primitive, unsignedInteger) {
    auto t = type::unsignedInteger().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("unsigned int"));
}

TEST(Primitive, signedLong) {
    auto t = type::signedLong().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("long"));
}

TEST(Primitive, unsignedLong) {
    auto t = type::unsignedLong().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("unsigned long"));
}

TEST(Primitive, signedInt128) {
    auto t = type::signedInt128().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(16));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("__int128"));
}

TEST(Primitive, unsignedInt128) {
    auto t = type::unsignedInt128().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(16));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("unsigned __int128"));
}

TEST(Primitive, floating) {
    auto t = type::floating().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsTrue());
    EXPECT_THAT(t.to_string(), Eq("float"));
}

TEST(Primitive, doubleFloating) {
    auto t = type::doubleFloating().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsTrue());
    EXPECT_THAT(t.to_string(), Eq("double"));
}

TEST(Primitive, longDoubleFloating) {
    auto t = type::longDoubleFloating().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(16));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsTrue());
    EXPECT_THAT(t.isComplex(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("long double"));
}

TEST(Primitive, complexFloat) {
    auto t = type::complexFloat().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.getAlignment(), Eq(4));
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isComplex(), IsTrue());
    EXPECT_THAT(t.kind(), Eq(type::PrimitiveKind::ComplexFloat));
    EXPECT_THAT(t.to_string(), Eq("_Complex float"));
}

TEST(Primitive, complexDouble) {
    auto t = type::complexDouble().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(16));
    EXPECT_THAT(t.getAlignment(), Eq(8));
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isComplex(), IsTrue());
    EXPECT_THAT(t.kind(), Eq(type::PrimitiveKind::ComplexDouble));
    EXPECT_THAT(t.to_string(), Eq("_Complex double"));
}

TEST(Primitive, complexLongDouble) {
    auto t = type::complexLongDouble().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(32));
    EXPECT_THAT(t.getAlignment(), Eq(16));
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isComplex(), IsTrue());
    EXPECT_THAT(t.kind(), Eq(type::PrimitiveKind::ComplexLongDouble));
    EXPECT_THAT(t.to_string(), Eq("_Complex long double"));
}

} // namespace


TEST(Primitive, signedShort) {
    auto t = type::signedShort().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(2));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("short"));
}

TEST(Primitive, unsignedShort) {
    auto t = type::unsignedShort().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(2));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("unsigned short"));
}
