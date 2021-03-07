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
    EXPECT_THAT(t.to_string(), Eq("char"));
}

TEST(Primitive, unsignedCharacter) {
    auto t = type::unsignedCharacter().getPrimitive();

    EXPECT_THAT(t.getSize(), Eq(1));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("unsigned char"));
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
    EXPECT_THAT(t.to_string(), Eq("long double"));
}

} // namespace

