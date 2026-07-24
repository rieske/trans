#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Type.h"

#include <stdexcept>

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

TEST(Type, arrayOfIntHasElementTypeAndSize) {
    using namespace type;
    auto a = array(signedInteger(), 3);
    ASSERT_THAT(a.isArray(), IsTrue());
    EXPECT_THAT(a.isPointer(), IsFalse());
    EXPECT_THAT(a.getSize(), Eq(12));
    EXPECT_THAT(a.getArraySize(), Eq(3));
    EXPECT_THAT(a.getElementType().getSize(), Eq(4));
    EXPECT_THAT(a.to_string(), Eq("int[3]"));
}

TEST(Type, nestedArrayToStringOutsideIn) {
    using namespace type;
    // int a[2][3] — outer count 2, element int[3]
    auto a = array(array(signedInteger(), 3), 2);
    EXPECT_THAT(a.getSize(), Eq(24));
    EXPECT_THAT(a.to_string(), Eq("int[2][3]"));
}

TEST(Type, arrayOfFunctionPointersIsComplete) {
    using namespace type;
    auto fp = pointer(function(signedInteger(), {}));
    auto a = array(fp, 3);
    EXPECT_THAT(a.isArray(), IsTrue());
    EXPECT_THAT(a.getSize(), Eq(24));
}

TEST(Type, arrayRejectsBareFunctionElement) {
    using namespace type;
    EXPECT_THROW(array(function(signedInteger(), {}), 2), std::invalid_argument);
}

TEST(Type, pointerToArrayIsPointerNotArray) {
    using namespace type;
    auto a = array(signedInteger(), 3);
    auto p = pointer(a);
    EXPECT_THAT(p.isPointer(), IsTrue());
    EXPECT_THAT(p.isArray(), IsFalse());
    EXPECT_THAT(p.getSize(), Eq(8));
    auto peeled = p.dereference();
    EXPECT_THAT(peeled.isArray(), IsTrue());
    EXPECT_THAT(peeled.getArraySize(), Eq(3));
}

TEST(Type, arrayRejectsNegativeCount) {
    using namespace type;
    EXPECT_THROW(array(signedInteger(), -1), std::invalid_argument);
}

TEST(Type, arrayRejectsSizeOverflow) {
    using namespace type;
    // 4 * 536870913 > INT_MAX
    EXPECT_THROW(array(signedInteger(), 536870913), std::invalid_argument);
}

TEST(Type, arrayRejectsVoidElement) {
    using namespace type;
    EXPECT_THROW(array(voidType(), 3), std::invalid_argument);
}

TEST(Type, getElementTypeOnNonArrayThrows) {
    using namespace type;
    EXPECT_THROW(signedInteger().getElementType(), std::runtime_error);
    EXPECT_THROW(signedInteger().getArraySize(), std::runtime_error);
}

TEST(Type, structureMembersHaveOffsetsAndSize) {
    using namespace type;
    auto s = structure({
        {"x", signedInteger()},
        {"y", signedInteger()},
    });
    ASSERT_THAT(s.isStructure(), IsTrue());
    EXPECT_THAT(s.getSize(), Eq(8));
    int off = -1;
    EXPECT_THAT(s.memberOffset("x", off), IsTrue());
    EXPECT_THAT(off, Eq(0));
    EXPECT_THAT(s.memberOffset("y", off), IsTrue());
    EXPECT_THAT(off, Eq(4));
    Type mt = voidType();
    EXPECT_THAT(s.memberType("x", mt), IsTrue());
    EXPECT_THAT(mt.isPrimitive(), IsTrue());
    EXPECT_THAT(mt.getSize(), Eq(4));
    EXPECT_THAT(mt.getPrimitive().isSigned(), IsTrue());
    Type mty = voidType();
    EXPECT_THAT(s.memberType("y", mty), IsTrue());
    EXPECT_THAT(mty.getSize(), Eq(4));
    EXPECT_THAT(s.memberOffset("z", off), IsFalse());
}

TEST(Type, structureLayoutAlignsMixedMembers) {
    // SysV/amd64-style: char then int → offsets 0/4, size 8 (not packed 0/1/size 5).
    using namespace type;
    auto s = structure({
        {"c", signedCharacter()},
        {"i", signedInteger()},
    });
    ASSERT_THAT(s.isStructure(), IsTrue());
    EXPECT_THAT(s.getSize(), Eq(8));
    int off = -1;
    EXPECT_THAT(s.memberOffset("c", off), IsTrue());
    EXPECT_THAT(off, Eq(0));
    EXPECT_THAT(s.memberOffset("i", off), IsTrue());
    EXPECT_THAT(off, Eq(4));
}

TEST(Type, pointerToStructureIsPointerNotStructure) {
    using namespace type;
    auto s = structure({{"x", signedInteger()}});
    auto p = pointer(s);
    EXPECT_THAT(p.isPointer(), IsTrue());
    EXPECT_THAT(p.isStructure(), IsFalse());
    int off = -1;
    EXPECT_THAT(p.memberOffset("x", off), IsFalse());
    auto peeled = p.dereference();
    EXPECT_THAT(peeled.isStructure(), IsTrue());
    EXPECT_THAT(peeled.memberOffset("x", off), IsTrue());
    EXPECT_THAT(off, Eq(0));
}

} // namespace
