#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Primitive.h"

namespace {

using namespace testing;

TEST(Primitive, signedCharacter) {
    auto t = type::Primitive::signedCharacter();

    EXPECT_THAT(t.getSize(), Eq(1));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("char"));
}

TEST(Primitive, unsignedCharacter) {
    auto t = type::Primitive::unsignedCharacter();

    EXPECT_THAT(t.getSize(), Eq(1));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("unsigned char"));
}

TEST(Primitive, signedInteger) {
    auto t = type::Primitive::signedInteger();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("int"));
}

TEST(Primitive, unsignedInteger) {
    auto t = type::Primitive::unsignedInteger();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("unsigned int"));
}

TEST(Primitive, signedLong) {
    auto t = type::Primitive::signedLong();

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("long"));
}

TEST(Primitive, unsignedLong) {
    auto t = type::Primitive::unsignedLong();

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("unsigned long"));
}

TEST(Primitive, floating) {
    auto t = type::Primitive::floating();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsTrue());
    EXPECT_THAT(t.to_string(), Eq("float"));
}

TEST(Primitive, doubleFloating) {
    auto t = type::Primitive::doubleFloating();

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsTrue());
    EXPECT_THAT(t.to_string(), Eq("double"));
}

TEST(Primitive, longDoubleFloating) {
    auto t = type::Primitive::longDoubleFloating();

    EXPECT_THAT(t.getSize(), Eq(16));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsTrue());
    EXPECT_THAT(t.to_string(), Eq("long double"));
}

} // namespace

