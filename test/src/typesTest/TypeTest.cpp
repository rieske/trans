#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Type.h"

namespace {

using namespace testing;

TEST(Type, signedCharacter) {
    type::Type t = type::Type::signedCharacter();

    EXPECT_THAT(t.getSize(), Eq(1));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, unsignedCharacter) {
    type::Type t = type::Type::unsignedCharacter();

    EXPECT_THAT(t.getSize(), Eq(1));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, signedInteger) {
    type::Type t = type::Type::signedInteger();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, unsignedInteger) {
    type::Type t = type::Type::unsignedInteger();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, signedLong) {
    type::Type t = type::Type::signedLong();

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, unsignedLong) {
    type::Type t = type::Type::unsignedLong();

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsFalse());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, floating) {
    type::Type t = type::Type::floating();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsTrue());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, doubleFloating) {
    type::Type t = type::Type::doubleFloating();

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsTrue());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, longDoubleFloating) {
    type::Type t = type::Type::longDoubleFloating();

    EXPECT_THAT(t.getSize(), Eq(16));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsTrue());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, constantTest) {
    std::vector<TypeQualifier> qualifiers {TypeQualifier::CONST};

    EXPECT_THAT(type::Type::signedCharacter(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::signedCharacter(qualifiers).isVolatile(), IsFalse());

    EXPECT_THAT(type::Type::unsignedCharacter(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::unsignedCharacter(qualifiers).isVolatile(), IsFalse());

    EXPECT_THAT(type::Type::signedInteger(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::signedInteger(qualifiers).isVolatile(), IsFalse());

    EXPECT_THAT(type::Type::unsignedInteger(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::unsignedInteger(qualifiers).isVolatile(), IsFalse());

    EXPECT_THAT(type::Type::signedLong(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::signedLong(qualifiers).isVolatile(), IsFalse());

    EXPECT_THAT(type::Type::unsignedLong(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::unsignedLong(qualifiers).isVolatile(), IsFalse());

    EXPECT_THAT(type::Type::floating(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::floating(qualifiers).isVolatile(), IsFalse());

    EXPECT_THAT(type::Type::doubleFloating(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::doubleFloating(qualifiers).isVolatile(), IsFalse());

    EXPECT_THAT(type::Type::longDoubleFloating(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::longDoubleFloating(qualifiers).isVolatile(), IsFalse());
}

TEST(Type, volatileTest) {
    std::vector<TypeQualifier> qualifiers {TypeQualifier::VOLATILE};

    EXPECT_THAT(type::Type::signedCharacter(qualifiers).isConst(), IsFalse());
    EXPECT_THAT(type::Type::signedCharacter(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::unsignedCharacter(qualifiers).isConst(), IsFalse());
    EXPECT_THAT(type::Type::unsignedCharacter(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::signedInteger(qualifiers).isConst(), IsFalse());
    EXPECT_THAT(type::Type::signedInteger(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::unsignedInteger(qualifiers).isConst(), IsFalse());
    EXPECT_THAT(type::Type::unsignedInteger(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::signedLong(qualifiers).isConst(), IsFalse());
    EXPECT_THAT(type::Type::signedLong(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::unsignedLong(qualifiers).isConst(), IsFalse());
    EXPECT_THAT(type::Type::unsignedLong(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::floating(qualifiers).isConst(), IsFalse());
    EXPECT_THAT(type::Type::floating(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::doubleFloating(qualifiers).isConst(), IsFalse());
    EXPECT_THAT(type::Type::doubleFloating(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::longDoubleFloating(qualifiers).isConst(), IsFalse());
    EXPECT_THAT(type::Type::longDoubleFloating(qualifiers).isVolatile(), IsTrue());
}

TEST(Type, constVolatileTest) {
    std::vector<TypeQualifier> qualifiers {TypeQualifier::CONST, TypeQualifier::VOLATILE};

    EXPECT_THAT(type::Type::signedCharacter(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::signedCharacter(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::unsignedCharacter(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::unsignedCharacter(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::signedInteger(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::signedInteger(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::unsignedInteger(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::unsignedInteger(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::signedLong(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::signedLong(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::unsignedLong(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::unsignedLong(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::floating(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::floating(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::doubleFloating(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::doubleFloating(qualifiers).isVolatile(), IsTrue());

    EXPECT_THAT(type::Type::longDoubleFloating(qualifiers).isConst(), IsTrue());
    EXPECT_THAT(type::Type::longDoubleFloating(qualifiers).isVolatile(), IsTrue());
}

TEST(Type, pointerToSignedInteger) {
    type::Type t = type::Type::pointer(type::Type::signedInteger());

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    auto pointsTo = t.dereference();
    EXPECT_THAT(pointsTo.getSize(), Eq(4));
    //EXPECT_THAT(pointsTo == type::Type::signedInteger(), IsTrue());
}

TEST(Type, pointerToSignedCharacter) {
    type::Type t = type::Type::pointer(type::Type::signedCharacter());

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isSigned(), IsTrue());
    EXPECT_THAT(t.isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    auto pointsTo = t.dereference();
    EXPECT_THAT(pointsTo.getSize(), Eq(1));
}

} // namespace

