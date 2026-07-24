#include "gtest/gtest.h"

#include "types/ObjectAbi.h"
#include "types/Type.h"

namespace {

using namespace type::object_abi;

// Captures the formulas historically inlined in StackMachine, CodeGeneratingVisitor,
// and SemanticAnalysisVisitor (global multi-word flatten).

TEST(ObjectAbi, valueWordsMatchesStackMachine) {
    EXPECT_EQ(valueWords(0), 1);
    EXPECT_EQ(valueWords(1), 1);
    EXPECT_EQ(valueWords(7), 1);
    EXPECT_EQ(valueWords(8), 1);
    EXPECT_EQ(valueWords(9), 2);
    EXPECT_EQ(valueWords(16), 2);
    EXPECT_EQ(valueWords(17), 3);
    EXPECT_EQ(valueWords(24), 3);
}

TEST(ObjectAbi, dataWordsMatchesSemanticGlobalFlatten) {
    EXPECT_EQ(dataWords(0), 0);
    EXPECT_EQ(dataWords(-1), 0);
    EXPECT_EQ(dataWords(1), 1);
    EXPECT_EQ(dataWords(8), 1);
    EXPECT_EQ(dataWords(9), 2);
    EXPECT_EQ(dataWords(16), 2);
    EXPECT_EQ(dataWords(17), 3);
}

TEST(ObjectAbi, wordByteOffsetAndIndex) {
    EXPECT_EQ(wordByteOffset(0), 0);
    EXPECT_EQ(wordByteOffset(1), 8);
    EXPECT_EQ(wordByteOffset(2), 16);
    EXPECT_EQ(wordIndexAt(0), 0);
    EXPECT_EQ(wordIndexAt(7), 0);
    EXPECT_EQ(wordIndexAt(8), 1);
    EXPECT_EQ(wordIndexAt(15), 1);
    EXPECT_EQ(wordIndexAt(16), 2);
}

TEST(ObjectAbi, needsMemoryReturnThresholdIsSixteenBytes) {
    EXPECT_FALSE(needsMemoryReturn(0));
    EXPECT_FALSE(needsMemoryReturn(8));
    EXPECT_FALSE(needsMemoryReturn(16));
    EXPECT_TRUE(needsMemoryReturn(17));
    EXPECT_TRUE(needsMemoryReturn(24));
}

// Single policy at Call/StartProcedure emission: only aggregates larger than 16
// bytes use sret. Caller Retrieve carries an explicit memoryReturn bit (see
// RetrieveTest) — StackMachine must not re-derive from size alone.
TEST(ObjectAbi, typeNeedsMemoryReturnOnlyForLargeAggregates) {
    EXPECT_FALSE(typeNeedsMemoryReturn(type::signedInteger()));
    EXPECT_FALSE(typeNeedsMemoryReturn(type::signedLong()));
    // Small struct (8 bytes) stays in registers.
    auto small = type::structure({ { "a", type::signedLong() } });
    EXPECT_FALSE(typeNeedsMemoryReturn(small));
    // Large struct (>16) needs sret.
    auto large = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
            { "c", type::signedLong() },
    });
    EXPECT_GT(large.getSize(), REGISTER_RETURN_MAX_BYTES);
    EXPECT_TRUE(typeNeedsMemoryReturn(large));
    // Arrays are aggregates: large arrays also sret.
    auto bigArr = type::array(type::signedCharacter(), 24);
    EXPECT_TRUE(bigArr.isAggregate());
    EXPECT_TRUE(typeNeedsMemoryReturn(bigArr));
}

// Caller and callee must agree: variadic suppresses sret even for large aggregates.
TEST(ObjectAbi, typeNeedsMemoryReturnSuppressedWhenVariadic) {
    auto large = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
            { "c", type::signedLong() },
    });
    EXPECT_TRUE(typeNeedsMemoryReturn(large));
    EXPECT_TRUE(typeNeedsMemoryReturn(large, false));
    EXPECT_FALSE(typeNeedsMemoryReturn(large, true));
    EXPECT_FALSE(typeNeedsMemoryReturn(type::signedInteger(), true));
    EXPECT_FALSE(typeNeedsMemoryReturn(type::signedInteger(), false));
}

TEST(ObjectAbi, constants) {
    EXPECT_EQ(MACHINE_WORD_SIZE, 8);
    EXPECT_EQ(STACK_ALIGNMENT, 16);
    EXPECT_EQ(REGISTER_RETURN_MAX_BYTES, 16);
    EXPECT_EQ(REGISTER_RETURN_MAX_WORDS, 2);
    EXPECT_STREQ(SRET_SYMBOL_NAME, "__sret");
}

} // namespace
