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

    EXPECT_THAT(t.to_string(), Eq("char"));
}

TEST(Type, unsignedCharacter) {
    auto t = type::unsignedCharacter();

    EXPECT_THAT(t.getSize(), Eq(1));
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isSigned(), IsFalse());
    EXPECT_THAT(t.getPrimitive().isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("unsigned char"));
}

TEST(Type, signedInteger) {
    auto t = type::signedInteger();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isSigned(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("int"));
}

TEST(Type, unsignedInteger) {
    auto t = type::unsignedInteger();

    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isSigned(), IsFalse());
    EXPECT_THAT(t.getPrimitive().isFloating(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("unsigned int"));
}

TEST(Type, constantPrimitiveTest) {
    std::vector<type::Qualifier> qualifiers {type::Qualifier::CONST};
    auto t = type::signedInteger(qualifiers);

    EXPECT_THAT(t.isConst(), IsTrue());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("const int"));
}

TEST(Type, volatilePrimitiveTest) {
    std::vector<type::Qualifier> qualifiers {type::Qualifier::VOLATILE};
    auto t = type::signedInteger(qualifiers);

    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsTrue());

    EXPECT_THAT(t.to_string(), Eq("volatile int"));
}

TEST(Type, constVolatilePrimitiveTest) {
    std::vector<type::Qualifier> qualifiers {type::Qualifier::CONST, type::Qualifier::VOLATILE};
    auto t = type::signedInteger(qualifiers);

    EXPECT_THAT(t.isConst(), IsTrue());
    EXPECT_THAT(t.isVolatile(), IsTrue());

    EXPECT_THAT(t.to_string(), Eq("const volatile int"));
}

TEST(Type, pointerToSignedInteger) {
    auto signedInteger = type::signedInteger();
    type::Type t = type::pointer(signedInteger);

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isPointer(), IsTrue());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("int*"));

    auto pointsTo = t.dereference();
    EXPECT_THAT(pointsTo.getSize(), Eq(4));
}

TEST(Type, pointerToPointerToSignedInteger) {
    auto signedInteger = type::signedInteger();
    type::Type t = type::pointer(type::pointer(signedInteger));

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isPointer(), IsTrue());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    auto pointsTo = t.dereference();
    EXPECT_THAT(pointsTo.getSize(), Eq(8));
    EXPECT_THAT(pointsTo.isPointer(), IsTrue());

    EXPECT_THAT(pointsTo.dereference().getSize(), Eq(4));
    EXPECT_THAT(pointsTo.dereference().isPointer(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("int**"));
}

TEST(Type, pointerToSignedCharacter) {
    auto signedChar = type::signedCharacter();
    type::Type t = type::pointer(signedChar);

    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.isPointer(), IsTrue());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("char*"));

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

    EXPECT_THAT(t.to_string(), Eq("void"));
}

TEST(Type, noArgFunctionReturningVoid) {
    auto t = type::function(type::voidType());

    EXPECT_THAT(t.getSize(), Eq(0));
    EXPECT_THAT(t.isPrimitive(), IsFalse());
    EXPECT_THAT(t.isFunction(), IsTrue());
    EXPECT_THAT(t.getFunction().getReturnType().isVoid(), IsTrue());
    EXPECT_THAT(t.getFunction().getArguments().size(), Eq(0));
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("void()"));
}

TEST(Type, noArgFunctionReturningInt) {
    auto t = type::function(type::signedInteger());

    EXPECT_THAT(t.getSize(), Eq(0));
    EXPECT_THAT(t.isPrimitive(), IsFalse());
    EXPECT_THAT(t.isFunction(), IsTrue());
    EXPECT_THAT(t.getFunction().getReturnType().isPrimitive(), IsTrue());
    EXPECT_THAT(t.getFunction().getReturnType().getPrimitive().getSize(), Eq(4));
    EXPECT_THAT(t.getFunction().getArguments().size(), Eq(0));
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("int()"));
}

TEST(Type, functionReturningIntAcceptingInt) {
    auto t = type::function(type::signedInteger(), {type::signedInteger()});

    EXPECT_THAT(t.getSize(), Eq(0));
    EXPECT_THAT(t.isPrimitive(), IsFalse());
    EXPECT_THAT(t.isFunction(), IsTrue());
    EXPECT_THAT(t.getFunction().getReturnType().isPrimitive(), IsTrue());
    EXPECT_THAT(t.getFunction().getReturnType().getPrimitive().getSize(), Eq(4));
    EXPECT_THAT(t.getFunction().getArguments().size(), Eq(1));
    EXPECT_THAT(t.getFunction().getArguments().at(0).isPrimitive(), IsTrue());
    EXPECT_THAT(t.getFunction().getArguments().at(0).getSize(), Eq(4));
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("int(int)"));
}

TEST(Type, functionReturningIntAcceptingIntAndPointerToPointerToUnsignedLong) {
    auto t = type::function(type::signedInteger(), {type::signedInteger(), type::pointer(type::pointer(type::unsignedLong()))});

    EXPECT_THAT(t.getSize(), Eq(0));
    EXPECT_THAT(t.isPrimitive(), IsFalse());
    EXPECT_THAT(t.isFunction(), IsTrue());
    EXPECT_THAT(t.getFunction().getReturnType().isPrimitive(), IsTrue());
    EXPECT_THAT(t.getFunction().getReturnType().getPrimitive().getSize(), Eq(4));
    EXPECT_THAT(t.getFunction().getArguments().size(), Eq(2));

    EXPECT_THAT(t.to_string(), Eq("int(int, unsigned long**)"));
}

} // namespace

