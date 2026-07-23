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

// C11 anonymous union: empty-name nested aggregate; outer lookup adds base offset.
TEST(Type, anonymousUnionMemberLookupPreservesOffsets) {
    auto nested = type::unionType({
            { "i", type::signedInteger() },
            { "j", type::signedInteger() },
    });
    auto outer = type::structure({
            { "tag", type::signedInteger() },
            { "", nested },
    });

    int tagOff = -1;
    int iOff = -1;
    int jOff = -1;
    type::Type memberTy = type::voidType();
    ASSERT_TRUE(outer.memberOffset("tag", tagOff));
    ASSERT_TRUE(outer.memberOffset("i", iOff));
    ASSERT_TRUE(outer.memberOffset("j", jOff));
    ASSERT_TRUE(outer.memberType("i", memberTy));
    EXPECT_THAT(tagOff, Eq(0));
    EXPECT_THAT(iOff, Eq(jOff));
    EXPECT_THAT(iOff, Gt(tagOff));
    EXPECT_THAT(memberTy.getSize(), Eq(4));
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

TEST(Type, signedShort) {
    auto t = type::signedShort();

    EXPECT_THAT(t.getSize(), Eq(2));
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isSigned(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isFloating(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("short"));
}

TEST(Type, unsignedShort) {
    auto t = type::unsignedShort();

    EXPECT_THAT(t.getSize(), Eq(2));
    EXPECT_THAT(t.isPrimitive(), IsTrue());
    EXPECT_THAT(t.getPrimitive().isSigned(), IsFalse());
    EXPECT_THAT(t.to_string(), Eq("unsigned short"));
}

// short arrays pack tightly (C ABI / ctype tables), not word-strided.
TEST(Type, shortArrayStrideIsTwo) {
    auto t = type::array(type::unsignedShort(), 4);
    EXPECT_THAT(t.getSize(), Eq(8));
    EXPECT_THAT(t.getElementStride(), Eq(2));
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

TEST(Type, arrayOfSignedInteger) {
    auto t = type::array(type::signedInteger(), 10);

    EXPECT_THAT(t.isArray(), IsTrue());
    EXPECT_THAT(t.isPointer(), IsFalse());
    EXPECT_THAT(t.getArraySize(), Eq(10));
    EXPECT_THAT(t.getElementType().isPrimitive(), IsTrue());
    // System V: int arrays pack at natural size (4), not word-strided.
    // Required for SHA1_CTX.ihv[5] and other libc/git aggregates.
    EXPECT_THAT(t.getElementStride(), Eq(4));
    EXPECT_THAT(t.getSize(), Eq(40));
    EXPECT_THAT(t.to_string(), Eq("int[10]"));
}

// Char arrays must pack (stride 1) so they match C strings / memcpy layout
// (git FLEX_ALLOC_MEM flex char members).
TEST(Type, arrayOfSignedCharacterIsPacked) {
    auto t = type::array(type::signedCharacter(), 4);

    EXPECT_THAT(t.isArray(), IsTrue());
    EXPECT_THAT(t.getArraySize(), Eq(4));
    EXPECT_THAT(t.getElementStride(), Eq(1));
    EXPECT_THAT(t.getSize(), Eq(4));
    EXPECT_THAT(t.to_string(), Eq("char[4]"));
}

TEST(Type, arrayOfPointers) {
    auto t = type::array(type::pointer(type::signedInteger()), 4);

    EXPECT_THAT(t.isArray(), IsTrue());
    EXPECT_THAT(t.getElementType().isPointer(), IsTrue());
    EXPECT_THAT(t.getElementStride(), Eq(8));
    EXPECT_THAT(t.getSize(), Eq(32));
    EXPECT_THAT(t.to_string(), Eq("int*[4]"));
}

// int (*)[3]: pointer size is 8, but dereference must recover the full array size
// (used as outer stride for p[i][j] / multi-dim parameter decay).
TEST(Type, pointerToArrayPreservesPointeeSize) {
    auto row = type::array(type::signedInteger(), 3);
    EXPECT_THAT(row.getSize(), Eq(12));
    EXPECT_THAT(row.getElementStride(), Eq(4));

    auto ptr = type::pointer(row);
    EXPECT_THAT(ptr.isPointer(), IsTrue());
    EXPECT_THAT(ptr.isArray(), IsFalse());
    EXPECT_THAT(ptr.getSize(), Eq(8));
    EXPECT_THAT(ptr.to_string(), Eq("int[3]*"));

    auto pointee = ptr.dereference();
    EXPECT_THAT(pointee.isArray(), IsTrue());
    EXPECT_THAT(pointee.isPointer(), IsFalse());
    EXPECT_THAT(pointee.getArraySize(), Eq(3));
    EXPECT_THAT(pointee.getSize(), Eq(12));
    EXPECT_THAT(pointee.getElementStride(), Eq(4));
    EXPECT_THAT(pointee.getElementType().getSize(), Eq(4));
}

// SHA1_CTX-like prefix: uint32_t ihv[5] then char buffer[64] must match C ABI
// (ihv is 20 bytes; buffer at 28) so SHA1DCUpdate finds the internal buffer.
TEST(Type, intArrayInStructUsesNaturalLayout) {
    auto s = type::structure({
            { "total", type::unsignedLong() },
            { "ihv", type::array(type::unsignedInteger(), 5) },
            { "buffer", type::array(type::unsignedCharacter(), 64) },
            { "found_collision", type::signedInteger() },
    });
    int ihvOff = -1;
    int bufOff = -1;
    int foundOff = -1;
    ASSERT_TRUE(s.memberOffset("ihv", ihvOff));
    ASSERT_TRUE(s.memberOffset("buffer", bufOff));
    ASSERT_TRUE(s.memberOffset("found_collision", foundOff));
    EXPECT_THAT(ihvOff, Eq(8));
    EXPECT_THAT(bufOff, Eq(28));
    EXPECT_THAT(foundOff, Eq(92));
    EXPECT_THAT(s.getSize(), Eq(96));
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

TEST(Type, pointerToFunctionIsPointerNotFunction) {
    auto fn = type::function(type::signedInteger(), {type::signedInteger()});
    auto ptr = type::pointer(fn);

    EXPECT_THAT(ptr.isPointer(), IsTrue());
    EXPECT_THAT(ptr.isFunction(), IsFalse());
    EXPECT_THAT(ptr.getSize(), Eq(8));
    EXPECT_THAT(ptr.dereference().isFunction(), IsTrue());
    EXPECT_THAT(ptr.dereference().getFunction().getReturnType().getSize(), Eq(4));
    EXPECT_THAT(ptr.dereference().getFunction().getArguments().size(), Eq(1u));
    EXPECT_THAT(ptr.to_string(), Eq("int(int)*"));
}

// Pointer-to-struct keeps struct payload for dereference(), but must not report
// isStructure()/isRecord() (same pattern as isArray/isFunction). Brace init of
// pointer members used isStructure() and expanded the pointee, clobbering the stack.
TEST(Type, pointerToStructureIsPointerNotStructure) {
    auto st = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
    });
    auto ptr = type::pointer(st);

    EXPECT_THAT(ptr.isPointer(), IsTrue());
    EXPECT_THAT(ptr.kind(), Eq(type::TypeKind::Pointer));
    EXPECT_THAT(ptr.isStructure(), IsFalse());
    EXPECT_THAT(ptr.isRecord(), IsFalse());
    EXPECT_THAT(ptr.isAggregate(), IsFalse());
    EXPECT_THAT(ptr.getSize(), Eq(8));
    EXPECT_THAT(ptr.dereference().isStructure(), IsTrue());
    EXPECT_THAT(ptr.dereference().isRecord(), IsTrue());
    EXPECT_THAT(ptr.dereference().isAggregate(), IsTrue());
    EXPECT_THAT(ptr.dereference().kind(), Eq(type::TypeKind::Struct));
    EXPECT_THAT(ptr.dereference().getSize(), Eq(16));
}

TEST(Type, isAggregateCoversArrayStructAndUnion) {
    auto arr = type::array(type::signedInteger(), 4);
    auto st = type::structure({ { "a", type::signedInteger() } });
    auto u = type::unionType({ { "a", type::signedInteger() } });
    EXPECT_TRUE(arr.isAggregate());
    EXPECT_TRUE(st.isAggregate());
    EXPECT_TRUE(u.isAggregate());
    EXPECT_FALSE(type::signedInteger().isAggregate());
    EXPECT_FALSE(type::pointer(st).isAggregate());
    EXPECT_EQ(arr.kind(), type::TypeKind::Array);
    EXPECT_EQ(st.kind(), type::TypeKind::Struct);
    EXPECT_EQ(u.kind(), type::TypeKind::Union);
}

// System V AMD64 natural layout: two ints pack into 8 bytes (offsets 0,4).
// Word-per-member layout breaks libc struct stat (st_size at wrong offset).
TEST(Type, structureNaturalLayoutTwoInts) {
    auto st = type::structure({
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
    });
    int aOff = -1;
    int bOff = -1;
    ASSERT_TRUE(st.memberOffset("a", aOff));
    ASSERT_TRUE(st.memberOffset("b", bOff));
    EXPECT_THAT(aOff, Eq(0));
    EXPECT_THAT(bOff, Eq(4));
    EXPECT_THAT(st.getSize(), Eq(8));
}

// Prefix of Linux x86-64 struct stat: st_size must be at byte 48 for fstat ABI.
TEST(Type, structureNaturalLayoutStatSizeOffset) {
    auto st = type::structure({
            { "st_dev", type::unsignedLong() },
            { "st_ino", type::unsignedLong() },
            { "st_nlink", type::unsignedLong() },
            { "st_mode", type::unsignedInteger() },
            { "st_uid", type::unsignedInteger() },
            { "st_gid", type::unsignedInteger() },
            { "__pad0", type::signedInteger() },
            { "st_rdev", type::unsignedLong() },
            { "st_size", type::signedLong() },
            { "st_blksize", type::signedLong() },
            { "st_blocks", type::signedLong() },
    });
    int sizeOff = -1;
    int blocksOff = -1;
    ASSERT_TRUE(st.memberOffset("st_size", sizeOff));
    ASSERT_TRUE(st.memberOffset("st_blocks", blocksOff));
    EXPECT_THAT(sizeOff, Eq(48));
    EXPECT_THAT(blocksOff, Eq(64));
}

} // namespace


TEST(Type, variadicFunctionFlag) {
    auto t = type::function(type::signedInteger(), { type::signedInteger() }, true);
    EXPECT_TRUE(t.isFunction());
    EXPECT_TRUE(t.getFunction().isVariadic());
    EXPECT_THAT(t.getFunction().getArguments().size(), Eq(1u));
}

TEST(Type, nonVariadicFunctionFlag) {
    auto t = type::function(type::signedInteger(), { type::signedInteger() }, false);
    EXPECT_FALSE(t.getFunction().isVariadic());
}

TEST(Type, incompleteStructureCompletedLater) {
    auto st = type::incompleteStructure();
    EXPECT_TRUE(st.isStructure());
    EXPECT_FALSE(st.isCompleteStructure());
    type::completeStructure(st, {
            { "a", type::signedInteger() },
            { "b", type::signedInteger() },
    });
    EXPECT_TRUE(st.isCompleteStructure());
    EXPECT_THAT(st.getSize(), Eq(8));
    int bOff = -1;
    ASSERT_TRUE(st.memberOffset("b", bOff));
    EXPECT_THAT(bOff, Eq(4));
}

TEST(Type, incompleteStructureSelfPointerSharesBody) {
    auto st = type::incompleteStructure();
    auto ptr = type::pointer(st);
    type::completeStructure(st, {
            { "next", type::pointer(st) },
            { "val", type::signedInteger() },
    });
    // Pointer-to-incomplete completed: pointee sees same body identity.
    EXPECT_THAT(ptr.dereference().structureBodyIdentity(), Eq(st.structureBodyIdentity()));
    EXPECT_TRUE(ptr.dereference().isCompleteStructure());
}

TEST(Type, unionLayoutAllMembersAtZero) {
    auto u = type::unionType({
            { "i", type::signedInteger() },
            { "d", type::doubleFloating() },
            { "c", type::signedCharacter() },
    });
    EXPECT_TRUE(u.isUnion());
    EXPECT_TRUE(u.isRecord());
    EXPECT_FALSE(u.isStructure()); // unions are not structures
    EXPECT_TRUE(u.isAggregate());
    int iOff = -1, dOff = -1, cOff = -1;
    ASSERT_TRUE(u.memberOffset("i", iOff));
    ASSERT_TRUE(u.memberOffset("d", dOff));
    ASSERT_TRUE(u.memberOffset("c", cOff));
    EXPECT_THAT(iOff, Eq(0));
    EXPECT_THAT(dOff, Eq(0));
    EXPECT_THAT(cOff, Eq(0));
    EXPECT_THAT(u.getSize(), Eq(8)); // max of int/double/char with alignment
}

TEST(Type, arrayDecaysToPointerToElement) {
    auto arr = type::array(type::signedInteger(), 4);
    auto decayed = arr.decayArray();
    EXPECT_TRUE(decayed.isPointer());
    EXPECT_FALSE(decayed.isArray());
    EXPECT_THAT(decayed.dereference().getSize(), Eq(4));
}

TEST(Type, withoutTopLevelQualifiersDropsConstVolatile) {
    auto t = type::signedInteger({ type::Qualifier::CONST, type::Qualifier::VOLATILE });
    EXPECT_TRUE(t.isConst());
    EXPECT_TRUE(t.isVolatile());
    auto bare = t.withoutTopLevelQualifiers();
    EXPECT_FALSE(bare.isConst());
    EXPECT_FALSE(bare.isVolatile());
    EXPECT_TRUE(bare.isPrimitive());
}

TEST(Type, canAssignFromCompatiblePrimitives) {
    auto i = type::signedInteger();
    auto l = type::signedLong();
    auto f = type::floating();
    EXPECT_TRUE(i.canAssignFrom(i));
    EXPECT_TRUE(l.canAssignFrom(l));
    // Usual arithmetic conversions: any primitive accepts any primitive.
    EXPECT_TRUE(i.canAssignFrom(l));
    EXPECT_TRUE(l.canAssignFrom(i));
    EXPECT_TRUE(i.canAssignFrom(f));
    EXPECT_TRUE(f.canAssignFrom(i));
}

TEST(Type, canAssignFromPointersAndNullConstant) {
    auto pi = type::pointer(type::signedInteger());
    auto pc = type::pointer(type::signedCharacter());
    auto i = type::signedInteger();
    EXPECT_TRUE(pi.canAssignFrom(pi));
    EXPECT_TRUE(pi.canAssignFrom(pc));
    // Integer → pointer (null pointer constant / product scalar mix).
    EXPECT_TRUE(pi.canAssignFrom(i));
    // Array decays to pointer for the source.
    auto arr = type::array(type::signedInteger(), 4);
    EXPECT_TRUE(pi.canAssignFrom(arr));
}

TEST(Type, canAssignFromRejectsStructScalarMismatch) {
    auto st = type::structure({ { "a", type::signedInteger() } });
    auto i = type::signedInteger();
    EXPECT_FALSE(i.canAssignFrom(st));
    EXPECT_FALSE(st.canAssignFrom(i));
    EXPECT_FALSE(type::voidType().canAssignFrom(i));
    EXPECT_FALSE(i.canAssignFrom(type::voidType()));
}

TEST(Type, canAssignFromDecaysArrayDestinationForCompatibility) {
    // Member arrays keep getType() as T[N]; comparison typeCheck uses that.
    auto arr = type::array(type::signedCharacter(), 4);
    auto p = type::pointer(type::signedCharacter());
    EXPECT_TRUE(arr.canAssignFrom(p));
    EXPECT_TRUE(p.canAssignFrom(arr));
}

TEST(Type, canAssignFromPointerToStructUnionForSockaddrArg) {
    // glibc __CONST_SOCKADDR_ARG: transparent union of sockaddr pointer members.
    // Callers pass struct sockaddr *; the parameter type is a union.
    auto sockaddr = type::structure({ { "sa_family", type::signedShort() } });
    auto sockaddrPtr = type::pointer(sockaddr);
    auto sockaddrArgUnion = type::unionType({
            { "__sockaddr__", sockaddrPtr },
    });
    EXPECT_TRUE(sockaddrArgUnion.canAssignFrom(sockaddrPtr));
    EXPECT_TRUE(sockaddrPtr.canAssignFrom(sockaddrArgUnion));
    // Still reject bare scalar ↔ struct.
    EXPECT_FALSE(sockaddr.canAssignFrom(type::signedInteger()));
}

TEST(Type, canAssignFromSameStructBodyOnly) {
    auto st1 = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
    });
    auto st2 = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
    });
    EXPECT_TRUE(st1.canAssignFrom(st1));
    // Distinct structure() calls allocate distinct bodies even with same layout.
    EXPECT_FALSE(st1.canAssignFrom(st2));
}

TEST(Type, equivalentToIgnoresTopLevelQualifiersAndComparesStructure) {
    auto i = type::signedInteger();
    auto ci = type::signedInteger({ type::Qualifier::CONST });
    EXPECT_TRUE(i.equivalentTo(ci));
    EXPECT_TRUE(type::pointer(i).equivalentTo(type::pointer(ci)));
    EXPECT_FALSE(type::signedInteger().equivalentTo(type::unsignedInteger()));
    EXPECT_FALSE(type::array(i, 3).equivalentTo(type::array(i, 4)));
    EXPECT_TRUE(type::array(i, 3).equivalentTo(type::array(i, 3)));

    auto st1 = type::structure({ { "a", type::signedLong() } });
    auto st2 = type::structure({ { "a", type::signedLong() } });
    EXPECT_TRUE(st1.equivalentTo(st1));
    EXPECT_FALSE(st1.equivalentTo(st2));

    auto f1 = type::function(type::signedInteger(), { type::signedInteger() }, false);
    auto f2 = type::function(type::signedInteger(), { type::signedInteger({ type::Qualifier::CONST }) }, false);
    auto f3 = type::function(type::signedInteger(), { type::signedInteger() }, true);
    EXPECT_TRUE(f1.equivalentTo(f2));
    EXPECT_FALSE(f1.equivalentTo(f3));
}

TEST(Type, memberLookupFailsForUnknown) {
    auto st = type::structure({ { "a", type::signedInteger() } });
    int off = 0;
    type::Type ty = type::voidType();
    EXPECT_FALSE(st.memberOffset("nope", off));
    EXPECT_FALSE(st.memberType("nope", ty));
}

TEST(Type, multiWordStructSizeAndOffsets) {
    // Three longs: git-like small struct layout.
    auto st = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
            { "c", type::signedLong() },
    });
    int a = -1, b = -1, c = -1;
    ASSERT_TRUE(st.memberOffset("a", a));
    ASSERT_TRUE(st.memberOffset("b", b));
    ASSERT_TRUE(st.memberOffset("c", c));
    EXPECT_THAT(a, Eq(0));
    EXPECT_THAT(b, Eq(8));
    EXPECT_THAT(c, Eq(16));
    EXPECT_THAT(st.getSize(), Eq(24));
}

// SysV AMD64 __builtin_va_list is array-of-1 of the 24-byte tag.
TEST(Type, builtinVaListIsArrayOfOneTwentyFourByteTag) {
    auto tag = type::builtinVaListTagType();
    EXPECT_THAT(tag.isStructure(), IsTrue());
    EXPECT_THAT(tag.getSize(), Eq(24));
    int gp = -1, fp = -1, over = -1, reg = -1;
    ASSERT_TRUE(tag.memberOffset("gp_offset", gp));
    ASSERT_TRUE(tag.memberOffset("fp_offset", fp));
    ASSERT_TRUE(tag.memberOffset("overflow_arg_area", over));
    ASSERT_TRUE(tag.memberOffset("reg_save_area", reg));
    EXPECT_THAT(gp, Eq(0));
    EXPECT_THAT(fp, Eq(4));
    EXPECT_THAT(over, Eq(8));
    EXPECT_THAT(reg, Eq(16));

    auto list = type::builtinVaListType();
    EXPECT_THAT(list.isArray(), IsTrue());
    EXPECT_THAT(list.getArraySize(), Eq(1));
    EXPECT_THAT(list.getSize(), Eq(24));
    EXPECT_THAT(list.getElementType().getSize(), Eq(24));
    // Parameter form decays to pointer-to-tag.
    auto decayed = list.decayArray();
    EXPECT_THAT(decayed.isPointer(), IsTrue());
    EXPECT_THAT(decayed.getSize(), Eq(8));
}
