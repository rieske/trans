#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Type.h"

namespace {

using namespace testing;

TEST(Type, signedCharacter) {
    auto t = type::signedCharacter();

    EXPECT_THAT(t.getSize(), Eq(1));
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isSigned(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, unsignedCharacter) {
    auto t = type::unsignedCharacter();

    EXPECT_THAT(t.getSize(), Eq(1));
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isSigned(), IsFalse());
    EXPECT_THAT(t.getPrimitive().isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, signedInteger) {
    auto t = type::signedInteger();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isSigned(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, unsignedInteger) {
    auto t = type::unsignedInteger();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isSigned(), IsFalse());
    EXPECT_THAT(t.getPrimitive().isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, constantPrimitiveTest) {
    std::vector<TypeQualifier> qualifiers {TypeQualifier::CONST};
    auto t = type::signedInteger(qualifiers);

    EXPECT_THAT(t.isConst(), IsTrue());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, volatilePrimitiveTest) {
    std::vector<TypeQualifier> qualifiers {TypeQualifier::VOLATILE};
    auto t = type::signedInteger(qualifiers);

    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsTrue());
}

TEST(Type, constVolatilePrimitiveTest) {
    std::vector<TypeQualifier> qualifiers {TypeQualifier::CONST, TypeQualifier::VOLATILE};
    auto t = type::signedInteger(qualifiers);

    EXPECT_THAT(t.isConst(), IsTrue());
    EXPECT_THAT(t.isVolatile(), IsTrue());
}

TEST(Type, pointerToSignedInteger) {
    auto signedInteger = type::signedInteger();
    type::Type t = type::pointer(signedInteger);

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isPointer(), IsTrue());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    auto pointsTo = t.dereference();
    EXPECT_THAT(pointsTo.getSize(), Eq(4));
}

TEST(Type, pointerToSignedCharacter) {
    auto signedChar = type::signedCharacter();
    type::Type t = type::pointer(signedChar);

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isPointer(), IsTrue());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    auto pointsTo = t.dereference();
    EXPECT_THAT(pointsTo.getSize(), Eq(1));
}

TEST(Type, voidType) {
    auto t = type::voidType();

    EXPECT_THAT(t.getSize(), Eq(0));
    EXPECT_THAT(t.isVoid(), IsTrue());
    EXPECT_THAT(t.isPrimitive(), IsFalse());
    EXPECT_THAT(t.isFunction(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, noArgFunctionReturningVoid) {
    auto t = type::function(type::voidType());

    EXPECT_THAT(t.getSize(), Eq(0));
    EXPECT_THAT(t.isPrimitive(), IsFalse());
    EXPECT_THAT(t.isFunction(), IsTrue());
    EXPECT_THAT(t.getReturnType().isVoid(), IsTrue());
    EXPECT_THAT(t.getArguments().size(), Eq(0));
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, noArgFunctionReturningInt) {
    auto t = type::function(type::signedInteger());

    EXPECT_THAT(t.getSize(), Eq(0));
    EXPECT_THAT(t.isPrimitive(), IsFalse());
    EXPECT_THAT(t.isFunction(), IsTrue());
    EXPECT_THAT(t.getReturnType().isPrimitive(), IsTrue());
    EXPECT_THAT(t.getReturnType().getPrimitive().getSize(), Eq(4));
    EXPECT_THAT(t.getArguments().size(), Eq(0));
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

TEST(Type, functionReturningIntAcceptingInt) {
    auto t = type::function(type::signedInteger(), {type::signedInteger()});

    EXPECT_THAT(t.getSize(), Eq(0));
    EXPECT_THAT(t.isPrimitive(), IsFalse());
    EXPECT_THAT(t.isFunction(), IsTrue());
    EXPECT_THAT(t.getReturnType().isPrimitive(), IsTrue());
    EXPECT_THAT(t.getReturnType().getPrimitive().getSize(), Eq(4));
    EXPECT_THAT(t.getArguments().size(), Eq(1));
    EXPECT_THAT(t.getArguments().at(0).isPrimitive(), IsTrue());
    EXPECT_THAT(t.getArguments().at(0).getSize(), Eq(4));
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
}

} // namespace

