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
    EXPECT_THAT(t.isPrimitive(), IsFalse());
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("int*"));

    auto pointsTo = t.dereference();
    EXPECT_THAT(pointsTo.getSize(), Eq(4));
    EXPECT_THAT(pointsTo.isPrimitive(), IsTrue());
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
    EXPECT_THAT(t.getFunction().argumentCount(), Eq(0u));
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
    EXPECT_THAT(t.getFunction().argumentCount(), Eq(0u));
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
    EXPECT_THAT(t.getFunction().argumentCount(), Eq(1u));
    EXPECT_THAT(t.getFunction().getArguments().at(0).isPrimitive(), IsTrue());
    EXPECT_THAT(t.getFunction().getArguments().at(0).getSize(), Eq(4));
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());

    EXPECT_THAT(t.to_string(), Eq("int(int)"));
}

TEST(Type, builtinVaListIsTwentyFourByteArrayOfTag) {
    auto tag = type::builtinVaListTagType();
    auto list = type::builtinVaListType();

    EXPECT_TRUE(tag.isStructure());
    EXPECT_THAT(tag.getSize(), Eq(24));
    EXPECT_TRUE(list.isArray());
    EXPECT_THAT(list.getArraySize(), Eq(1));
    EXPECT_THAT(list.getSize(), Eq(24));
    EXPECT_TRUE(list.getElementType().isStructure());
    EXPECT_THAT(list.getElementType().getSize(), Eq(24));

    int gpOff = -1;
    ASSERT_TRUE(tag.memberOffset("gp_offset", gpOff));
    EXPECT_THAT(gpOff, Eq(0));
    int fpOff = -1;
    ASSERT_TRUE(tag.memberOffset("fp_offset", fpOff));
    EXPECT_THAT(fpOff, Eq(4));
    int overflowOff = -1;
    ASSERT_TRUE(tag.memberOffset("overflow_arg_area", overflowOff));
    EXPECT_THAT(overflowOff, Eq(8));
    int saveOff = -1;
    ASSERT_TRUE(tag.memberOffset("reg_save_area", saveOff));
    EXPECT_THAT(saveOff, Eq(16));
}

TEST(Type, variadicFunctionFlag) {
    auto plain = type::function(type::signedInteger(), { type::signedInteger() });
    auto var = type::function(type::signedInteger(), { type::signedInteger() }, true);

    EXPECT_FALSE(plain.getFunction().isVariadic());
    EXPECT_TRUE(var.getFunction().isVariadic());
    EXPECT_THAT(var.to_string(), Eq("int(int, ...)"));
    EXPECT_FALSE(plain.equivalentTo(var));
}

TEST(Type, functionReturningIntAcceptingIntAndPointerToPointerToUnsignedLong) {
    auto t = type::function(type::signedInteger(), {type::signedInteger(), type::pointer(type::pointer(type::unsignedLong()))});

    EXPECT_THAT(t.getSize(), Eq(0));
    EXPECT_THAT(t.isPrimitive(), IsFalse());
    EXPECT_THAT(t.isFunction(), IsTrue());
    EXPECT_THAT(t.getFunction().getReturnType().isPrimitive(), IsTrue());
    EXPECT_THAT(t.getFunction().getReturnType().getPrimitive().getSize(), Eq(4));
    EXPECT_THAT(t.getFunction().getArguments().size(), Eq(2));
    EXPECT_THAT(t.getFunction().argumentCount(), Eq(2u));

    EXPECT_THAT(t.to_string(), Eq("int(int, unsigned long**)"));
}

TEST(Type, arrayOfIntHasElementTypeAndSize) {
    using namespace type;
    auto a = array(signedInteger(), 3);
    ASSERT_THAT(a.isArray(), IsTrue());
    EXPECT_THAT(a.isPointer(), IsFalse());
    EXPECT_THAT(a.isIncompleteArray(), IsFalse());
    EXPECT_THAT(a.getSize(), Eq(12));
    EXPECT_THAT(a.getArraySize(), Eq(3));
    EXPECT_THAT(a.getElementType().getSize(), Eq(4));
    EXPECT_THAT(a.to_string(), Eq("int[3]"));
}

TEST(Type, incompleteArrayIsDistinctFromZeroLength) {
    using namespace type;
    auto inc = incompleteArray(signedInteger());
    auto zero = array(signedInteger(), 0);
    ASSERT_THAT(inc.isArray(), IsTrue());
    EXPECT_THAT(inc.isIncompleteArray(), IsTrue());
    EXPECT_THAT(zero.isIncompleteArray(), IsFalse());
    EXPECT_THAT(inc.getSize(), Eq(0));
    EXPECT_THAT(inc.to_string(), Eq("int[]"));
    EXPECT_THAT(zero.to_string(), Eq("int[0]"));
    EXPECT_FALSE(inc.equivalentTo(zero));
    EXPECT_TRUE(inc.equivalentTo(incompleteArray(signedInteger())));
}

TEST(Type, arrayRejectsIncompleteArrayElement) {
    using namespace type;
    EXPECT_THROW(array(incompleteArray(signedInteger()), 2), std::invalid_argument);
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
    EXPECT_THROW(signedInteger().getElementType(), std::domain_error);
    EXPECT_THROW(signedInteger().getArraySize(), std::domain_error);
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

TEST(Type, structureTrailingPadAndArrayStride) {
    // int then char needs trailing pad to align 4 → size 8; array stride multiplies that.
    using namespace type;
    auto s = structure({
        {"i", signedInteger()},
        {"c", signedCharacter()},
    });
    ASSERT_THAT(s.isStructure(), IsTrue());
    EXPECT_THAT(s.getSize(), Eq(8));
    int off = -1;
    EXPECT_THAT(s.memberOffset("i", off), IsTrue());
    EXPECT_THAT(off, Eq(0));
    EXPECT_THAT(s.memberOffset("c", off), IsTrue());
    EXPECT_THAT(off, Eq(4));
    EXPECT_THAT(array(s, 2).getSize(), Eq(16));
}

TEST(Type, structureRejectsIncompleteMembers) {
    using namespace type;
    EXPECT_THROW(structure({{"v", voidType()}}), std::invalid_argument);
    EXPECT_THROW(structure({{"f", function(signedInteger(), {})}}), std::invalid_argument);
}

TEST(Type, structureAllowsFunctionPointerMembers) {
    using namespace type;
    auto s = structure({{"fp", pointer(function(signedInteger(), {}))}});
    EXPECT_THAT(s.isStructure(), IsTrue());
    EXPECT_THAT(s.getSize(), Eq(8));
}

TEST(Type, structureRejectsDuplicateMemberNames) {
    using namespace type;
    EXPECT_THROW(
            structure({{"x", signedInteger()}, {"x", signedCharacter()}}),
            std::invalid_argument);
}

TEST(Type, structureRejectsSizeOverflow) {
    using namespace type;
    // Each member fits in int; sum of two exceeds INT_MAX.
    auto huge = array(signedCharacter(), 1073741824); // 2^30
    EXPECT_THROW(structure({{"a", huge}, {"b", huge}}), std::invalid_argument);
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

// Recursive Type contracts (Phase 0.1): pointer is its own kind, not a payload bleed.

TEST(Type, pointerToFunctionIsNotBareFunction) {
    using namespace type;
    auto fn = function(signedInteger(), { signedInteger() });
    auto pfn = pointer(fn);

    EXPECT_THAT(fn.isFunction(), IsTrue());
    EXPECT_THAT(fn.isPointer(), IsFalse());
    EXPECT_THAT(pfn.isPointer(), IsTrue());
    EXPECT_THAT(pfn.isFunction(), IsFalse());
    EXPECT_THAT(pfn.dereference().isFunction(), IsTrue());
    EXPECT_THAT(pfn.getSize(), Eq(8));
}

TEST(Type, pointerToArrayOfPointersToFunction) {
    using namespace type;
    auto pfn = pointer(function(voidType(), {}));
    auto arr = array(pfn, 4);
    auto p = pointer(arr);

    EXPECT_THAT(p.isPointer(), IsTrue());
    EXPECT_THAT(p.isArray(), IsFalse());
    EXPECT_THAT(p.isFunction(), IsFalse());
    auto peeledArr = p.dereference();
    EXPECT_THAT(peeledArr.isArray(), IsTrue());
    EXPECT_THAT(peeledArr.getArraySize(), Eq(4));
    EXPECT_THAT(peeledArr.getElementType().isPointer(), IsTrue());
    EXPECT_THAT(peeledArr.getElementType().dereference().isFunction(), IsTrue());
}

TEST(Type, incompleteStructureSharedBodyCompletesInPlace) {
    using namespace type;
    auto tag = incompleteStructure();
    ASSERT_THAT(tag.isStructure(), IsTrue());
    EXPECT_THAT(tag.isIncompleteStructure(), IsTrue());
    EXPECT_THAT(tag.getSize(), Eq(0));

    // Pointer and alias must observe the same completed layout (self-ref tags).
    auto alias = tag;
    auto ptr = pointer(tag);
    completeStructure(tag, { { "x", signedInteger() }, { "next", pointer(tag) } });

    EXPECT_THAT(tag.isIncompleteStructure(), IsFalse());
    EXPECT_THAT(tag.getSize(), Eq(16)); // int @0 + pad + pointer @8
    EXPECT_THAT(alias.isIncompleteStructure(), IsFalse());
    EXPECT_THAT(alias.getSize(), Eq(16));
    EXPECT_THAT(alias.structureBodyIdentity(), Eq(tag.structureBodyIdentity()));

    auto peeled = ptr.dereference();
    EXPECT_THAT(peeled.isStructure(), IsTrue());
    EXPECT_THAT(peeled.isIncompleteStructure(), IsFalse());
    int off = -1;
    EXPECT_THAT(peeled.memberOffset("x", off), IsTrue());
    EXPECT_THAT(off, Eq(0));
    EXPECT_THAT(peeled.memberOffset("next", off), IsTrue());
    EXPECT_THAT(off, Eq(8));
}

TEST(Type, memberCountAndMemberAtMatchLayout) {
    using namespace type;
    auto s = structure({
        { "c", signedCharacter() },
        { "i", signedInteger() },
    });
    EXPECT_THAT(s.memberCount(), Eq(2));
    std::string name;
    Type mt = voidType();
    int off = -1;
    EXPECT_THAT(s.memberAt(0, name, mt, off), IsTrue());
    EXPECT_THAT(name, Eq("c"));
    EXPECT_THAT(off, Eq(0));
    EXPECT_THAT(s.memberAt(1, name, mt, off), IsTrue());
    EXPECT_THAT(name, Eq("i"));
    EXPECT_THAT(off, Eq(4));
    EXPECT_THAT(s.memberAt(2, name, mt, off), IsFalse());
}

TEST(Type, kindClassifiesNodesWithoutPayloadBleed) {
    using namespace type;
    EXPECT_THAT(voidType().kind(), Eq(TypeKind::Void));
    EXPECT_THAT(signedInteger().kind(), Eq(TypeKind::Primitive));
    EXPECT_THAT(pointer(signedInteger()).kind(), Eq(TypeKind::Pointer));
    EXPECT_THAT(function(voidType(), {}).kind(), Eq(TypeKind::Function));
    EXPECT_THAT(array(signedInteger(), 2).kind(), Eq(TypeKind::Array));
    EXPECT_THAT(structure({ { "x", signedInteger() } }).kind(), Eq(TypeKind::Struct));
    EXPECT_THAT(incompleteStructure().kind(), Eq(TypeKind::Struct));
}

TEST(Type, longDoubleAlignmentIsSize) {
    using namespace type;
    EXPECT_THAT(longDoubleFloating().getAlignment(), Eq(16));
    EXPECT_THAT(signedLong().getAlignment(), Eq(8));
    EXPECT_THAT(signedInteger().getAlignment(), Eq(4));
    auto s = structure({ { "ld", longDoubleFloating() } });
    EXPECT_THAT(s.getAlignment(), Eq(16));
    EXPECT_THAT(s.getSize(), Eq(16));
}

TEST(Type, completeStructureRejectsNonRecord) {
    using namespace type;
    Type i = signedInteger();
    EXPECT_THROW(completeStructure(i, { { "x", signedInteger() } }), std::domain_error);
    EXPECT_THROW(completeUnion(i, { { "x", signedInteger() } }), std::domain_error);
}

TEST(Type, arrayRejectsIncompleteRecordElement) {
    using namespace type;
    EXPECT_THROW(array(incompleteRecord(), 3), std::invalid_argument);
    EXPECT_THROW(array(incompleteStructure(), 1), std::invalid_argument);
}

TEST(Type, pointerAppliesQualifiersViaConstructor) {
    using namespace type;
    auto p = pointer(signedInteger(), { Qualifier::CONST, Qualifier::VOLATILE });
    EXPECT_THAT(p.isConst(), IsTrue());
    EXPECT_THAT(p.isVolatile(), IsTrue());
    EXPECT_THAT(p.isPointer(), IsTrue());
}

TEST(Type, restrictQualifierIsParsedAndDiscarded) {
    using namespace type;
    auto t = signedInteger({ Qualifier::RESTRICT });
    EXPECT_THAT(t.isConst(), IsFalse());
    EXPECT_THAT(t.isVolatile(), IsFalse());
    auto p = pointer(signedInteger(), { Qualifier::RESTRICT, Qualifier::CONST });
    EXPECT_THAT(p.isConst(), IsTrue());
    EXPECT_THAT(p.isVolatile(), IsFalse());
    EXPECT_THAT(p.isPointer(), IsTrue());
}

TEST(Type, emptyCompleteRecordArrayHasZeroSize) {
    using namespace type;
    auto empty = structure({});
    EXPECT_THAT(empty.isCompleteRecord(), IsTrue());
    EXPECT_THAT(empty.getSize(), Eq(0));
    auto a = array(empty, 3);
    EXPECT_THAT(a.getSize(), Eq(0));
    EXPECT_THAT(a.getElementStride(), Eq(0));
}

TEST(Type, unionLayoutAllMembersAtZero) {
    using namespace type;
    auto u = unionType({
        { "c", signedCharacter() },
        { "i", signedInteger() },
    });
    EXPECT_THAT(u.isUnion(), IsTrue());
    EXPECT_THAT(u.kind(), Eq(TypeKind::Union));
    EXPECT_THAT(u.getSize(), Eq(4));
    int off = -1;
    EXPECT_THAT(u.memberOffset("c", off), IsTrue());
    EXPECT_THAT(off, Eq(0));
    EXPECT_THAT(u.memberOffset("i", off), IsTrue());
    EXPECT_THAT(off, Eq(0));
}

TEST(Type, completeStructureFailurePreservesPriorSharedLayout) {
    using namespace type;
    auto tag = incompleteStructure();
    completeStructure(tag, { { "x", signedInteger() } });
    EXPECT_THAT(tag.getSize(), Eq(4));
    auto alias = tag;
    auto ptr = pointer(tag);

    EXPECT_THROW(completeStructure(tag, { { "y", incompleteStructure() } }), std::invalid_argument);

    EXPECT_THAT(tag.getSize(), Eq(4));
    EXPECT_THAT(alias.getSize(), Eq(4));
    EXPECT_THAT(ptr.dereference().getSize(), Eq(4));
    int off = -1;
    EXPECT_THAT(tag.memberOffset("x", off), IsTrue());
    EXPECT_THAT(off, Eq(0));
    EXPECT_THAT(tag.isCompleteRecord(), IsTrue());
}

TEST(Type, structureNamedPredicatesAreStructOnly) {
    using namespace type;
    auto s = structure({ { "x", signedInteger() } });
    EXPECT_THAT(s.isCompleteStructure(), IsTrue());
    EXPECT_THAT(s.isCompleteRecord(), IsTrue());

    auto u = unionType({ { "x", signedInteger() } });
    EXPECT_THAT(u.isCompleteRecord(), IsTrue());
    EXPECT_THAT(u.isCompleteStructure(), IsFalse());
    EXPECT_THAT(u.isIncompleteStructure(), IsFalse());

    auto inc = incompleteStructure();
    EXPECT_THAT(inc.isIncompleteStructure(), IsTrue());
    EXPECT_THAT(inc.isIncompleteRecord(), IsTrue());
}

TEST(Type, equivalentToIgnoresTopLevelQualifiers) {
    using namespace type;
    auto a = signedInteger({ Qualifier::CONST });
    auto b = signedInteger();
    EXPECT_THAT(a.equivalentTo(b), IsTrue());
    EXPECT_THAT(pointer(a).equivalentTo(pointer(b)), IsTrue());
    EXPECT_THAT(pointer(a).equivalentTo(signedInteger()), IsFalse());
}

TEST(Type, signedShortFactory) {
    using namespace type;
    auto t = signedShort();
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getSize(), Eq(2));
    EXPECT_THAT(t.to_string(), Eq("short"));
}

} // namespace

