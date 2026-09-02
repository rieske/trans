#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "types/Type.h"

#include <memory>
#include <stdexcept>

namespace {

using namespace testing;

int offsetOf(const type::Type& t, const std::string& name) {
    auto m = type::lookupMember(t, name);
    if (!m) {
        ADD_FAILURE() << "no member " << name;
        return -1;
    }
    return m->offsetBytes;
}

bool hasMember(const type::Type& t, const std::string& name) {
    return type::lookupMember(t, name).has_value();
}

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

TEST(Type, booleanIsDistinctFromUnsignedCharacter) {
    auto b = type::boolean();
    auto c = type::unsignedCharacter();
    EXPECT_THAT(b.getSize(), Eq(1));
    EXPECT_THAT(b.isPrimitive(), IsTrue());
    EXPECT_THAT(b.getPrimitive().isBoolean(), IsTrue());
    EXPECT_THAT(b.to_string(), Eq("bool"));
    EXPECT_THAT(b.equivalentTo(c), IsFalse());
    EXPECT_THAT(b.sameQualifiedType(c), IsFalse());
    EXPECT_THAT(b.sameQualifiedType(type::boolean()), IsTrue());
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

    EXPECT_THAT(offsetOf(tag, "gp_offset"), Eq(0));
    EXPECT_THAT(offsetOf(tag, "fp_offset"), Eq(4));
    EXPECT_THAT(offsetOf(tag, "overflow_arg_area"), Eq(8));
    EXPECT_THAT(offsetOf(tag, "reg_save_area"), Eq(16));
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

TEST(Type, variableArrayIsCompleteAndNotSized) {
    using namespace type;
    auto va = variableArray(signedInteger());
    ASSERT_THAT(va.isArray(), IsTrue());
    EXPECT_THAT(va.isIncompleteArray(), IsFalse());
    EXPECT_THAT(va.isVariableArray(), IsTrue());
    EXPECT_THAT(array(signedInteger(), 3).isVariableArray(), IsFalse());
    EXPECT_THAT(incompleteArray(signedInteger()).isVariableArray(), IsFalse());
    EXPECT_THAT(va.to_string(), Eq("int[*]"));
    EXPECT_TRUE(va.equivalentTo(variableArray(signedInteger())));
    EXPECT_FALSE(va.equivalentTo(array(signedInteger(), 0)));
    EXPECT_FALSE(va.equivalentTo(incompleteArray(signedInteger())));
}

TEST(Type, variableArrayCompatibleWithSizedAndIncomplete) {
    using namespace type;
    auto va = variableArray(signedInteger());
    EXPECT_TRUE(va.compatibleWith(array(signedInteger(), 3)));
    EXPECT_TRUE(va.compatibleWith(incompleteArray(signedInteger())));
    EXPECT_TRUE(array(signedInteger(), 3).compatibleWith(va));
}

TEST(Type, variableArrayCompositePrefersKnownBound) {
    using namespace type;
    auto va = variableArray(signedInteger());
    auto three = array(signedInteger(), 3);
    auto merged = va.composite(three);
    ASSERT_TRUE(merged.has_value());
    EXPECT_FALSE(merged->isVariableArray());
    EXPECT_THAT(merged->getArraySize(), Eq(3));
    auto bothVa = va.composite(variableArray(signedInteger()));
    ASSERT_TRUE(bothVa.has_value());
    EXPECT_TRUE(bothVa->isVariableArray());
}

TEST(Type, variableArrayCompositePrefersSpecifiedBound) {
    using namespace type;
    auto specifiedId = std::make_shared<VlaBound>();
    auto specified = variableArray(pointer(array(signedInteger(), 3)), specifiedId);
    auto unspecified = variableArray(pointer(incompleteArray(signedInteger())));
    auto fromUnspec = unspecified.composite(specified);
    ASSERT_TRUE(fromUnspec.has_value());
    EXPECT_EQ(fromUnspec->vlaBound().get(), specifiedId.get());
    EXPECT_FALSE(fromUnspec->vlaBound()->unspecified);
    EXPECT_EQ(fromUnspec->getElementType().dereference().getArraySize(), 3);
    auto fromSpec = specified.composite(unspecified);
    ASSERT_TRUE(fromSpec.has_value());
    EXPECT_EQ(fromSpec->vlaBound().get(), specifiedId.get());
    EXPECT_EQ(fromSpec->getElementType().dereference().getArraySize(), 3);
}

TEST(Type, variableArrayRejectsIncompleteElement) {
    using namespace type;
    EXPECT_THROW(variableArray(incompleteArray(signedInteger())), std::invalid_argument);
    EXPECT_THROW(variableArray(voidType()), std::invalid_argument);
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

TEST(Type, memberCopySharesPayload) {
    using namespace type;
    Type::Member original { "x", signedInteger(), 0 };
    Type::Member copy = original;
    ASSERT_NE(original.type.get(), nullptr);
    EXPECT_EQ(original.type.get(), copy.type.get());
    EXPECT_TRUE(original.type->equivalentTo(signedInteger()));
    EXPECT_THAT(copy.name, Eq("x"));
    EXPECT_THAT(copy.offsetBytes, Eq(0));
}

TEST(Type, structureMembersHaveOffsetsAndSize) {
    using namespace type;
    auto s = structure({
        {"x", signedInteger()},
        {"y", signedInteger()},
    });
    ASSERT_THAT(s.isStructure(), IsTrue());
    EXPECT_THAT(s.getSize(), Eq(8));
    EXPECT_THAT(offsetOf(s, "x"), Eq(0));
    EXPECT_THAT(offsetOf(s, "y"), Eq(4));
    auto mx = lookupMember(s, "x");
    ASSERT_TRUE(mx);
    EXPECT_THAT(mx->type.isPrimitive(), IsTrue());
    EXPECT_THAT(mx->type.getSize(), Eq(4));
    EXPECT_THAT(mx->type.getPrimitive().isSigned(), IsTrue());
    auto my = lookupMember(s, "y");
    ASSERT_TRUE(my);
    EXPECT_THAT(my->type.getSize(), Eq(4));
    EXPECT_FALSE(hasMember(s, "z"));
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
    EXPECT_THAT(offsetOf(s, "c"), Eq(0));
    EXPECT_THAT(offsetOf(s, "i"), Eq(4));
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
    EXPECT_THAT(offsetOf(s, "i"), Eq(0));
    EXPECT_THAT(offsetOf(s, "c"), Eq(4));
    EXPECT_THAT(array(s, 2).getSize(), Eq(16));
}

TEST(Type, bitFieldPackingMatchesGcc) {
    using namespace type;
    auto p = incompleteRecord();
    completeStructure(p, {
            MemberSpec { "a", signedInteger(), 3 },
            MemberSpec { "b", signedInteger(), 5 },
    });
    EXPECT_THAT(p.getSize(), Eq(4));
    auto ba = lookupMember(p, "a");
    ASSERT_TRUE(ba);
    ASSERT_TRUE(ba->isBitField());
    EXPECT_THAT(ba->bitField->width, Eq(3));
    EXPECT_THAT(ba->bitField->shift, Eq(0));
    EXPECT_THAT(ba->offsetBytes, Eq(0));
    auto bb = lookupMember(p, "b");
    ASSERT_TRUE(bb);
    ASSERT_TRUE(bb->isBitField());
    EXPECT_THAT(bb->bitField->width, Eq(5));
    EXPECT_THAT(bb->bitField->shift, Eq(3));

    auto s = incompleteRecord();
    completeStructure(s, {
            MemberSpec { "a", signedInteger(), 31 },
            MemberSpec { "b", signedInteger(), 2 },
    });
    EXPECT_THAT(s.getSize(), Eq(8));
    auto sb = lookupMember(s, "b");
    ASSERT_TRUE(sb);
    ASSERT_TRUE(sb->isBitField());
    EXPECT_THAT(sb->bitField->shift, Eq(0));
    EXPECT_THAT(sb->offsetBytes, Eq(4));

    auto c = incompleteRecord();
    completeStructure(c, {
            MemberSpec { "a", signedCharacter(), 1 },
            MemberSpec { "b", signedCharacter(), 1 },
    });
    EXPECT_THAT(c.getSize(), Eq(1));

    auto z = incompleteRecord();
    completeStructure(z, {
            MemberSpec { "", signedInteger(), 0 },
            MemberSpec { "x", signedInteger(), 1 },
    });
    EXPECT_THAT(z.getSize(), Eq(4));
    EXPECT_THAT(z.memberCount(), Eq(1));
    auto zx = lookupMember(z, "x");
    ASSERT_TRUE(zx);
    ASSERT_TRUE(zx->isBitField());
    EXPECT_THAT(zx->bitField->shift, Eq(0));

    auto u = incompleteRecord();
    completeStructure(u, {
            MemberSpec { "", signedInteger(), 7 },
            MemberSpec { "x", signedInteger(), 1 },
    });
    EXPECT_THAT(u.getSize(), Eq(4));
    EXPECT_THAT(u.memberCount(), Eq(1));
    auto ux = lookupMember(u, "x");
    ASSERT_TRUE(ux);
    ASSERT_TRUE(ux->isBitField());
    EXPECT_THAT(ux->bitField->shift, Eq(7));
    EXPECT_THAT(ux->offsetBytes, Eq(0));

    auto mix = incompleteRecord();
    completeStructure(mix, {
            MemberSpec { "a", signedInteger(), 1 },
            MemberSpec { "c", signedCharacter() },
    });
    EXPECT_THAT(mix.getSize(), Eq(4));
    EXPECT_THAT(offsetOf(mix, "c"), Eq(1));

    auto un = incompleteRecord();
    completeUnion(un, {
            MemberSpec { "a", signedInteger(), 3 },
            MemberSpec { "b", signedInteger(), 5 },
    });
    EXPECT_THAT(un.getSize(), Eq(4));
    auto ua = lookupMember(un, "a");
    ASSERT_TRUE(ua);
    ASSERT_TRUE(ua->isBitField());
    EXPECT_THAT(ua->bitField->shift, Eq(0));
    auto ub = lookupMember(un, "b");
    ASSERT_TRUE(ub);
    ASSERT_TRUE(ub->isBitField());
    EXPECT_THAT(ub->bitField->shift, Eq(0));

    auto ll = incompleteRecord();
    completeStructure(ll, {
            MemberSpec { "a", signedLong(), 40 },
            MemberSpec { "b", signedInteger(), 8 },
    });
    EXPECT_THAT(ll.getSize(), Eq(8));
    EXPECT_THAT(offsetOf(ll, "a"), Eq(0));
    EXPECT_THAT(offsetOf(ll, "b"), Eq(4));
    auto llb = lookupMember(ll, "b");
    ASSERT_TRUE(llb);
    ASSERT_TRUE(llb->isBitField());
    EXPECT_THAT(llb->bitField->shift, Eq(8));
}

TEST(Type, bitFieldMask) {
    using namespace type;
    EXPECT_THAT(bitFieldMask(0), Eq(0ull));
    EXPECT_THAT(bitFieldMask(-1), Eq(0ull));
    EXPECT_THAT(bitFieldMask(1), Eq(1ull));
    EXPECT_THAT(bitFieldMask(3), Eq(7ull));
    EXPECT_THAT(bitFieldMask(8), Eq(0xffull));
    EXPECT_THAT(bitFieldMask(64), Eq(~0ull));
    EXPECT_THAT(bitFieldMask(65), Eq(~0ull));
}

TEST(Type, resolveOffsetof) {
    using namespace type;
    auto s = incompleteRecord();
    completeStructure(s, {
            MemberSpec { "a", signedInteger(), 3 },
            MemberSpec { "b", signedInteger() },
    });
    auto offB = resolveOffsetof(s, "b");
    EXPECT_THAT(offB.status, Eq(OffsetofStatus::Ok));
    EXPECT_THAT(offB.offsetBytes, Eq(4));

    auto offA = resolveOffsetof(s, "a");
    EXPECT_THAT(offA.status, Eq(OffsetofStatus::BitField));

    auto offMissing = resolveOffsetof(s, "nope");
    EXPECT_THAT(offMissing.status, Eq(OffsetofStatus::Missing));

    auto incomplete = incompleteRecord();
    auto offInc = resolveOffsetof(incomplete, "x");
    EXPECT_THAT(offInc.status, Eq(OffsetofStatus::Incomplete));
}

TEST(Type, bitFieldRejectsIllegalWidthAndType) {
    using namespace type;
    auto badWide = incompleteRecord();
    EXPECT_THROW(completeStructure(badWide, { MemberSpec { "x", signedInteger(), 33 } }),
            std::invalid_argument);
    auto badZero = incompleteRecord();
    EXPECT_THROW(completeStructure(badZero, { MemberSpec { "x", signedInteger(), 0 } }),
            std::invalid_argument);
    auto badType = incompleteRecord();
    EXPECT_THROW(completeStructure(badType, { MemberSpec { "f", floating(), 3 } }),
            std::invalid_argument);
    auto badInt128 = incompleteRecord();
    EXPECT_THROW(completeStructure(badInt128, { MemberSpec { "x", signedInt128(), 8 } }),
            std::invalid_argument);
    auto badInt128Width = incompleteRecord();
    EXPECT_THROW(completeStructure(badInt128Width, { MemberSpec { "x", signedInt128(), 80 } }),
            std::invalid_argument);
}

TEST(Type, structureRejectsIncompleteMembers) {
    using namespace type;
    EXPECT_THROW(structure({{"v", voidType()}}), std::invalid_argument);
    EXPECT_THROW(structure({{"f", function(signedInteger(), {})}}), std::invalid_argument);
}

TEST(Type, structureAllowsFlexibleArrayMember) {
    using namespace type;
    auto s = structure({
            { "n", signedInteger() },
            { "data", incompleteArray(signedInteger()) },
    });
    EXPECT_THAT(s.isStructure(), IsTrue());
    EXPECT_THAT(s.isIncompleteRecord(), IsFalse());
    EXPECT_THAT(s.getSize(), Eq(4));
    EXPECT_THAT(offsetOf(s, "data"), Eq(4));
    auto data = lookupMember(s, "data");
    ASSERT_TRUE(data);
    EXPECT_THAT(data->type.isIncompleteArray(), IsTrue());
}

TEST(Type, structureFlexibleArrayAfterCharPadsToElementAlign) {
    using namespace type;
    auto s = structure({
            { "c", signedCharacter() },
            { "data", incompleteArray(signedInteger()) },
    });
    EXPECT_THAT(s.getSize(), Eq(4));
    EXPECT_THAT(offsetOf(s, "data"), Eq(4));
}

TEST(Type, structureRejectsFlexibleArrayIfNotLast) {
    using namespace type;
    EXPECT_THROW(structure({
            { "data", incompleteArray(signedInteger()) },
            { "n", signedInteger() },
    }), std::invalid_argument);
}

TEST(Type, structureRejectsFlexibleArrayAsOnlyMember) {
    using namespace type;
    EXPECT_THROW(structure({
            { "data", incompleteArray(signedInteger()) },
    }), std::invalid_argument);
}

TEST(Type, unionRejectsFlexibleArrayMember) {
    using namespace type;
    EXPECT_THROW(unionType({
            { "n", signedInteger() },
            { "data", incompleteArray(signedInteger()) },
    }), std::invalid_argument);
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
    EXPECT_FALSE(hasMember(p, "x"));
    auto peeled = p.dereference();
    EXPECT_THAT(peeled.isStructure(), IsTrue());
    EXPECT_THAT(offsetOf(peeled, "x"), Eq(0));
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
    EXPECT_THAT(tag.isIncompleteRecord(), IsTrue());
    EXPECT_THAT(tag.getSize(), Eq(0));

    // Pointer and alias must observe the same completed layout (self-ref tags).
    auto alias = tag;
    auto ptr = pointer(tag);
    completeStructure(tag, {
            MemberSpec { "x", signedInteger() },
            MemberSpec { "next", pointer(tag) },
    });

    EXPECT_THAT(tag.isIncompleteRecord(), IsFalse());
    EXPECT_THAT(tag.getSize(), Eq(16)); // int @0 + pad + pointer @8
    EXPECT_THAT(alias.isIncompleteRecord(), IsFalse());
    EXPECT_THAT(alias.getSize(), Eq(16));
    EXPECT_THAT(alias.structureBodyIdentity(), Eq(tag.structureBodyIdentity()));

    auto peeled = ptr.dereference();
    EXPECT_THAT(peeled.isStructure(), IsTrue());
    EXPECT_THAT(peeled.isIncompleteRecord(), IsFalse());
    EXPECT_THAT(offsetOf(peeled, "x"), Eq(0));
    EXPECT_THAT(offsetOf(peeled, "next"), Eq(8));
}

TEST(Type, memberCountAndMemberAtMatchLayout) {
    using namespace type;
    auto s = structure({
        { "c", signedCharacter() },
        { "i", signedInteger() },
    });
    EXPECT_THAT(s.memberCount(), Eq(2));
    auto m0 = memberAt(s, 0);
    ASSERT_TRUE(m0);
    EXPECT_THAT(m0->name, Eq("c"));
    EXPECT_THAT(m0->offsetBytes, Eq(0));
    auto m1 = memberAt(s, 1);
    ASSERT_TRUE(m1);
    EXPECT_THAT(m1->name, Eq("i"));
    EXPECT_THAT(m1->offsetBytes, Eq(4));
    EXPECT_FALSE(memberAt(s, 2));
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
    EXPECT_THAT(signedInt128().getAlignment(), Eq(16));
    EXPECT_THAT(unsignedInt128().getSize(), Eq(16));
    EXPECT_THAT(signedInteger().getAlignment(), Eq(4));
    auto s = structure({ { "ld", longDoubleFloating() } });
    EXPECT_THAT(s.getAlignment(), Eq(16));
    EXPECT_THAT(s.getSize(), Eq(16));
}

TEST(Type, complexAlignmentIsCorrespondingReal) {
    using namespace type;
    EXPECT_THAT(complexFloat().getSize(), Eq(8));
    EXPECT_THAT(complexFloat().getAlignment(), Eq(4));
    EXPECT_THAT(complexDouble().getSize(), Eq(16));
    EXPECT_THAT(complexDouble().getAlignment(), Eq(8));
    EXPECT_THAT(complexLongDouble().getSize(), Eq(32));
    EXPECT_THAT(complexLongDouble().getAlignment(), Eq(16));
    auto wrapped = structure({ { "c", signedCharacter() }, { "z", complexFloat() } });
    EXPECT_THAT(wrapped.getAlignment(), Eq(4));
    EXPECT_THAT(wrapped.getSize(), Eq(12));
}

TEST(Type, completeStructureRejectsNonRecord) {
    using namespace type;
    Type i = signedInteger();
    EXPECT_THROW(completeStructure(i, { MemberSpec { "x", signedInteger() } }), std::domain_error);
    EXPECT_THROW(completeUnion(i, { MemberSpec { "x", signedInteger() } }), std::domain_error);
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
    EXPECT_THAT(offsetOf(u, "c"), Eq(0));
    EXPECT_THAT(offsetOf(u, "i"), Eq(0));
}

TEST(Type, completeStructureFailurePreservesPriorSharedLayout) {
    using namespace type;
    auto tag = incompleteStructure();
    completeStructure(tag, { MemberSpec { "x", signedInteger() } });
    EXPECT_THAT(tag.getSize(), Eq(4));
    auto alias = tag;
    auto ptr = pointer(tag);

    EXPECT_THROW(completeStructure(tag, { MemberSpec { "y", incompleteStructure() } }),
            std::invalid_argument);

    EXPECT_THAT(tag.getSize(), Eq(4));
    EXPECT_THAT(alias.getSize(), Eq(4));
    EXPECT_THAT(ptr.dereference().getSize(), Eq(4));
    EXPECT_THAT(offsetOf(tag, "x"), Eq(0));
    EXPECT_THAT(tag.isCompleteRecord(), IsTrue());
}

TEST(Type, structureNamedPredicatesAreStructOnly) {
    using namespace type;
    auto s = structure({ { "x", signedInteger() } });
    EXPECT_THAT(s.isStructure(), IsTrue());
    EXPECT_THAT(s.isCompleteRecord(), IsTrue());
    EXPECT_THAT(s.isUnion(), IsFalse());

    auto u = unionType({ { "x", signedInteger() } });
    EXPECT_THAT(u.isUnion(), IsTrue());
    EXPECT_THAT(u.isCompleteRecord(), IsTrue());
    EXPECT_THAT(u.isStructure(), IsFalse());

    auto inc = incompleteStructure();
    EXPECT_THAT(inc.isStructure(), IsTrue());
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

TEST(Type, sameUnqualifiedTypeDropsOnlyTopLevelQualifiers) {
    using namespace type;
    auto i = signedInteger();
    auto ci = signedInteger({ Qualifier::CONST });
    EXPECT_THAT(i.sameUnqualifiedType(i), IsTrue());
    EXPECT_THAT(i.sameUnqualifiedType(ci), IsTrue());
    EXPECT_THAT(ci.sameUnqualifiedType(i), IsTrue());
    EXPECT_THAT(pointer(i).sameUnqualifiedType(pointer(i)), IsTrue());
    EXPECT_THAT(pointer(i).sameUnqualifiedType(pointer(ci)), IsFalse());
    EXPECT_THAT(pointer(ci).sameUnqualifiedType(pointer(i)), IsFalse());
    EXPECT_THAT(array(i, 3).sameUnqualifiedType(pointer(i)), IsFalse());
}

TEST(Type, indexElementPeelsPointerOrArray) {
    using namespace type;
    auto i = signedInteger();
    auto peeledPointer = pointer(i).indexElement();
    ASSERT_TRUE(peeledPointer.has_value());
    EXPECT_THAT(peeledPointer->sameQualifiedType(i), IsTrue());

    auto peeledArray = array(i, 3).indexElement();
    ASSERT_TRUE(peeledArray.has_value());
    EXPECT_THAT(peeledArray->sameQualifiedType(i), IsTrue());

    EXPECT_FALSE(i.indexElement().has_value());
}

TEST(Type, sameQualifiedTypeRespectsQualifiersAtEachLevel) {
    using namespace type;
    auto i = signedInteger();
    auto ci = signedInteger({ Qualifier::CONST });
    EXPECT_THAT(i.sameQualifiedType(i), IsTrue());
    EXPECT_THAT(i.sameQualifiedType(ci), IsFalse());
    EXPECT_THAT(ci.sameQualifiedType(i), IsFalse());
    EXPECT_THAT(pointer(i).sameQualifiedType(pointer(i)), IsTrue());
    EXPECT_THAT(pointer(i).sameQualifiedType(pointer(ci)), IsFalse());
    EXPECT_THAT(pointer(ci).sameQualifiedType(pointer(i)), IsFalse());
    EXPECT_THAT(pointer(i).equivalentTo(pointer(ci)), IsTrue());
    EXPECT_THAT(incompleteArray(i).sameQualifiedType(array(i, 3)), IsFalse());
}

TEST(Type, compatibleWithIncompleteAndCompleteArray) {
    using namespace type;
    auto i = signedInteger();
    auto inc = incompleteArray(i);
    auto three = array(i, 3);
    auto two = array(i, 2);
    EXPECT_THAT(inc.compatibleWith(three), IsTrue());
    EXPECT_THAT(three.compatibleWith(inc), IsTrue());
    EXPECT_THAT(inc.compatibleWith(inc), IsTrue());
    EXPECT_THAT(three.compatibleWith(array(i, 3)), IsTrue());
    EXPECT_THAT(three.compatibleWith(two), IsFalse());
    EXPECT_THAT(inc.sameQualifiedType(three), IsFalse());

    auto ci = signedInteger({ Qualifier::CONST });
    EXPECT_THAT(incompleteArray(i).compatibleWith(array(ci, 1)), IsFalse());
    EXPECT_THAT(incompleteArray(ci).compatibleWith(array(ci, 1)), IsTrue());
}

TEST(Type, compatibleWithNestedArrayAndPointerToArray) {
    using namespace type;
    auto i = signedInteger();
    auto incRows = incompleteArray(array(i, 3));
    auto twoByThree = array(array(i, 3), 2);
    auto twoByFour = array(array(i, 4), 2);
    EXPECT_THAT(incRows.compatibleWith(twoByThree), IsTrue());
    EXPECT_THAT(incRows.compatibleWith(twoByFour), IsFalse());

    auto pInc = pointer(incompleteArray(i));
    auto pFour = pointer(array(i, 4));
    EXPECT_THAT(pInc.compatibleWith(pFour), IsTrue());
    EXPECT_THAT(pInc.compatibleWith(pointer(array(i, 3))), IsTrue());
    EXPECT_THAT(pointer(array(i, 4)).compatibleWith(pointer(array(i, 3))), IsFalse());
}

TEST(Type, compositePrefersCompleteArrayBound) {
    using namespace type;
    auto i = signedInteger();
    auto inc = incompleteArray(i);
    auto three = array(i, 3);

    auto fromInc = inc.composite(three);
    ASSERT_TRUE(fromInc.has_value());
    EXPECT_THAT(fromInc->sameQualifiedType(three), IsTrue());
    EXPECT_THAT(fromInc->getArraySize(), Eq(3));

    auto fromComplete = three.composite(inc);
    ASSERT_TRUE(fromComplete.has_value());
    EXPECT_THAT(fromComplete->sameQualifiedType(three), IsTrue());

    EXPECT_FALSE(three.composite(array(i, 2)).has_value());

    auto bothInc = inc.composite(incompleteArray(i));
    ASSERT_TRUE(bothInc.has_value());
    EXPECT_THAT(bothInc->isIncompleteArray(), IsTrue());
}

TEST(Type, compositeNestedArrayAndPointerToArray) {
    using namespace type;
    auto i = signedInteger();
    auto twoByThree = array(array(i, 3), 2);
    auto merged = incompleteArray(array(i, 3)).composite(twoByThree);
    ASSERT_TRUE(merged.has_value());
    EXPECT_THAT(merged->sameQualifiedType(twoByThree), IsTrue());

    auto pFour = pointer(array(i, 4));
    auto pMerged = pointer(incompleteArray(i)).composite(pFour);
    ASSERT_TRUE(pMerged.has_value());
    EXPECT_THAT(pMerged->sameQualifiedType(pFour), IsTrue());
}

TEST(Type, withQualifiersSetsConstAndVolatile) {
    using namespace type;
    auto i = signedInteger();
    auto c = i.withQualifiers({ Qualifier::CONST });
    EXPECT_THAT(c.isConst(), IsTrue());
    EXPECT_THAT(c.isVolatile(), IsFalse());
    EXPECT_THAT(c.to_string(), Eq("const int"));
    EXPECT_THAT(i.isConst(), IsFalse());

    auto cv = c.withQualifiers({ Qualifier::VOLATILE });
    EXPECT_THAT(cv.isConst(), IsTrue());
    EXPECT_THAT(cv.isVolatile(), IsTrue());
    EXPECT_THAT(cv.to_string(), Eq("const volatile int"));

    auto p = pointer(signedCharacter()).withQualifiers({ Qualifier::CONST });
    EXPECT_THAT(p.isConst(), IsTrue());
    EXPECT_THAT(p.isPointer(), IsTrue());
    EXPECT_THAT(p.dereference().isConst(), IsFalse());

    auto rec = structure({ { "x", signedInteger() } }).withQualifiers({ Qualifier::CONST });
    EXPECT_THAT(rec.isConst(), IsTrue());
    EXPECT_THAT(rec.isCompleteRecord(), IsTrue());
}

TEST(Type, signedShortFactory) {
    using namespace type;
    auto t = signedShort();
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getSize(), Eq(2));
    EXPECT_THAT(t.to_string(), Eq("short"));
}

} // namespace

