#include "gtest/gtest.h"

#include "types/SysVClassify.h"
#include "types/Type.h"

namespace {

using type::sysv::Class;
using type::sysv::Classification;
using type::sysv::classify;
using type::sysv::integerEightbytes;
using type::sysv::isInteger;
using type::sysv::isSse;
using type::sysv::isX87;
using type::sysv::sseEightbytes;

void expectRegs(const Classification& c, std::initializer_list<Class> want) {
    ASSERT_FALSE(c.memory);
    ASSERT_EQ(c.count, static_cast<int>(want.size()));
    int i = 0;
    for (Class cls : want) {
        EXPECT_EQ(c.eightbytes[static_cast<std::size_t>(i)], cls) << "eightbyte " << i;
        ++i;
    }
}

TEST(SysVClassify, classPredicates) {
    EXPECT_TRUE(isInteger(Class::Integer));
    EXPECT_FALSE(isInteger(Class::Sse));
    EXPECT_FALSE(isInteger(Class::SseUp));
    EXPECT_FALSE(isInteger(Class::X87));
    EXPECT_TRUE(isSse(Class::Sse));
    EXPECT_TRUE(isSse(Class::SseUp));
    EXPECT_FALSE(isSse(Class::Integer));
    EXPECT_FALSE(isSse(Class::X87));
    EXPECT_TRUE(isX87(Class::X87));
    EXPECT_TRUE(isX87(Class::X87Up));
    EXPECT_TRUE(isX87(Class::ComplexX87));
    EXPECT_FALSE(isX87(Class::Sse));
    EXPECT_FALSE(isX87(Class::Integer));
}

TEST(SysVClassify, scalars) {
    expectRegs(classify(type::signedInteger()), { Class::Integer });
    expectRegs(classify(type::signedLong()), { Class::Integer });
    expectRegs(classify(type::boolean()), { Class::Integer });
    expectRegs(classify(type::pointer(type::signedInteger())), { Class::Integer });
    expectRegs(classify(type::floating()), { Class::Sse });
    expectRegs(classify(type::doubleFloating()), { Class::Sse });
}

TEST(SysVClassify, int128IsTwoIntegerEightbytes) {
    const auto s = classify(type::signedInt128());
    expectRegs(s, { Class::Integer, Class::Integer });
    EXPECT_TRUE(s.inRegisters());
    EXPECT_EQ(s.alignBytes, 16);
    EXPECT_EQ(s.gprExtend, type::sysv::GprExtend::None);
    EXPECT_EQ(integerEightbytes(s), 2);
    const auto u = classify(type::unsignedInt128());
    expectRegs(u, { Class::Integer, Class::Integer });
    EXPECT_EQ(u.alignBytes, 16);
    EXPECT_EQ(u.gprExtend, type::sysv::GprExtend::None);
}

TEST(SysVClassify, narrowIntegerGprExtend) {
    using type::sysv::GprExtend;
    EXPECT_EQ(classify(type::unsignedCharacter()).gprExtend, GprExtend::Zero);
    EXPECT_EQ(classify(type::signedCharacter()).gprExtend, GprExtend::Sign);
    EXPECT_EQ(classify(type::boolean()).gprExtend, GprExtend::Zero);
    EXPECT_EQ(classify(type::unsignedShort()).gprExtend, GprExtend::Zero);
    EXPECT_EQ(classify(type::signedShort()).gprExtend, GprExtend::Sign);
    EXPECT_EQ(classify(type::unsignedInteger()).gprExtend, GprExtend::Zero);
    EXPECT_EQ(classify(type::signedInteger()).gprExtend, GprExtend::Sign);
    EXPECT_EQ(classify(type::signedLong()).gprExtend, GprExtend::None);
    EXPECT_EQ(classify(type::unsignedLong()).gprExtend, GprExtend::None);
    EXPECT_EQ(classify(type::pointer(type::signedInteger())).gprExtend, GprExtend::None);
    auto boxedChar = type::structure({ { "c", type::unsignedCharacter() } });
    EXPECT_EQ(classify(boxedChar).gprExtend, GprExtend::None);
}

TEST(SysVClassify, complexFloatIsSse) {
    const auto c = classify(type::complexFloat());
    expectRegs(c, { Class::Sse });
    EXPECT_TRUE(c.inRegisters());
    EXPECT_FALSE(c.hasX87());
    EXPECT_FALSE(type::sysv::isComplexX87(c));
    EXPECT_EQ(c.alignBytes, 4);
    EXPECT_EQ(sseEightbytes(c), 1);
}

TEST(SysVClassify, complexDoubleIsTwoSse) {
    const auto c = classify(type::complexDouble());
    expectRegs(c, { Class::Sse, Class::Sse });
    EXPECT_TRUE(c.inRegisters());
    EXPECT_FALSE(c.hasX87());
    EXPECT_EQ(c.alignBytes, 8);
    EXPECT_EQ(sseEightbytes(c), 2);
}

TEST(SysVClassify, complexLongDoubleIsComplexX87) {
    const auto c = classify(type::complexLongDouble());
    expectRegs(c, { Class::ComplexX87 });
    EXPECT_FALSE(c.inRegisters());
    EXPECT_TRUE(c.hasX87());
    EXPECT_TRUE(type::sysv::isComplexX87(c));
    EXPECT_FALSE(c.memory);
    EXPECT_EQ(c.alignBytes, 16);
}

TEST(SysVClassify, structOfComplexLongDoubleIsMemory) {
    auto s = type::structure({ { "z", type::complexLongDouble() } });
    EXPECT_EQ(s.getSize(), 32);
    EXPECT_TRUE(classify(s).memory);
    EXPECT_FALSE(type::sysv::isComplexX87(classify(s)));
}

TEST(SysVClassify, structOfComplexFloatIsSse) {
    auto s = type::structure({ { "z", type::complexFloat() } });
    expectRegs(classify(s), { Class::Sse });
    EXPECT_EQ(s.getSize(), 8);
    EXPECT_EQ(s.getAlignment(), 4);
}

TEST(SysVClassify, longDoubleIsX87) {
    const auto c = classify(type::longDoubleFloating());
    expectRegs(c, { Class::X87, Class::X87Up });
    EXPECT_FALSE(c.inRegisters());
    EXPECT_TRUE(c.hasX87());
    EXPECT_EQ(c.alignBytes, 16);
}

TEST(SysVClassify, carriesTypeAlignment) {
    EXPECT_EQ(classify(type::signedInteger()).alignBytes, 4);
    EXPECT_EQ(classify(type::signedLong()).alignBytes, 8);
    auto threeLongs = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
            { "c", type::signedLong() },
    });
    EXPECT_TRUE(classify(threeLongs).memory);
    EXPECT_EQ(classify(threeLongs).alignBytes, 8);
    auto ldThenLong = type::structure({
            { "ld", type::longDoubleFloating() },
            { "n", type::signedLong() },
    });
    EXPECT_TRUE(classify(ldThenLong).memory);
    EXPECT_EQ(classify(ldThenLong).alignBytes, 16);
}

TEST(SysVClassify, twoLongsAreTwoIntegerEightbytes) {
    auto s = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
    });
    expectRegs(classify(s), { Class::Integer, Class::Integer });
    EXPECT_TRUE(classify(s).inRegisters());
    EXPECT_FALSE(classify(s).hasX87());
    EXPECT_EQ(integerEightbytes(classify(s)), 2);
    EXPECT_EQ(sseEightbytes(classify(s)), 0);
}

TEST(SysVClassify, structOfDoubleIsSse) {
    auto s = type::structure({ { "d", type::doubleFloating() } });
    expectRegs(classify(s), { Class::Sse });
    EXPECT_EQ(sseEightbytes(classify(s)), 1);
}

TEST(SysVClassify, mixedIntDouble) {
    auto s = type::structure({
            { "x", type::signedInteger() },
            { "y", type::doubleFloating() },
    });
    EXPECT_EQ(s.getSize(), 16);
    expectRegs(classify(s), { Class::Integer, Class::Sse });
    EXPECT_EQ(integerEightbytes(classify(s)), 1);
    EXPECT_EQ(sseEightbytes(classify(s)), 1);
}

TEST(SysVClassify, twoFloatsShareOneSseEightbyte) {
    auto s = type::structure({
            { "a", type::floating() },
            { "b", type::floating() },
    });
    EXPECT_EQ(s.getSize(), 8);
    expectRegs(classify(s), { Class::Sse });
}

TEST(SysVClassify, intThenFloatInOneEightbyteIsInteger) {
    auto s = type::structure({
            { "x", type::signedInteger() },
            { "y", type::floating() },
    });
    EXPECT_EQ(s.getSize(), 8);
    expectRegs(classify(s), { Class::Integer });
}

TEST(SysVClassify, threeLongsAreMemory) {
    auto s = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
            { "c", type::signedLong() },
    });
    EXPECT_TRUE(classify(s).memory);
    EXPECT_EQ(classify(s).count, 0);
}

TEST(SysVClassify, largeUnionIsMemory) {
    auto u = type::unionType({
            { "small", type::signedInteger() },
            { "big", type::array(type::signedLong(), 3) },
    });
    EXPECT_TRUE(classify(u).memory);
}

TEST(SysVClassify, unionIntDoubleIsInteger) {
    auto u = type::unionType({
            { "i", type::signedInteger() },
            { "d", type::doubleFloating() },
    });
    EXPECT_EQ(u.getSize(), 8);
    expectRegs(classify(u), { Class::Integer });
}

TEST(SysVClassify, voidAndIncompleteAreEmpty) {
    EXPECT_FALSE(classify(type::voidType()).memory);
    EXPECT_EQ(classify(type::voidType()).count, 0);
    EXPECT_FALSE(classify(type::incompleteRecord()).memory);
    EXPECT_EQ(classify(type::incompleteRecord()).count, 0);
}

TEST(SysVClassify, arrayDecaysNotClassifiedAsMemoryWhenPointer) {
    expectRegs(classify(type::pointer(type::array(type::signedLong(), 4))), { Class::Integer });
}

} // namespace
