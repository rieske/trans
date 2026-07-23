#include "gtest/gtest.h"

#include "codegen/ObjectAbi.h"

namespace {

using namespace codegen::object_abi;

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

TEST(ObjectAbi, aggregateNeedsMemoryReturnOnlyForAggregates) {
    EXPECT_FALSE(aggregateNeedsMemoryReturn(false, 24));
    EXPECT_FALSE(aggregateNeedsMemoryReturn(true, 16));
    EXPECT_TRUE(aggregateNeedsMemoryReturn(true, 17));
    EXPECT_TRUE(aggregateNeedsMemoryReturn(true, 24));
}

TEST(ObjectAbi, valueNeedsMemoryReturnMatchesWordsGreaterThanTwo) {
    EXPECT_FALSE(valueNeedsMemoryReturn(16));
    EXPECT_TRUE(valueNeedsMemoryReturn(17));
    EXPECT_EQ(valueNeedsMemoryReturn(17), valueWords(17) > REGISTER_RETURN_MAX_WORDS);
}

TEST(ObjectAbi, constants) {
    EXPECT_EQ(MACHINE_WORD_SIZE, 8);
    EXPECT_EQ(STACK_ALIGNMENT, 16);
    EXPECT_EQ(REGISTER_RETURN_MAX_BYTES, 16);
    EXPECT_EQ(REGISTER_RETURN_MAX_WORDS, 2);
    EXPECT_STREQ(SRET_SYMBOL_NAME, "__sret");
}

} // namespace
