#include "gtest/gtest.h"

#include "types/ObjectAbi.h"
#include "types/ObjectAbiType.h"
#include "types/Type.h"

namespace {

using namespace type::object_abi;

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

// SysV: complete records whose classify() is MEMORY. Scalars/arrays never sret.
TEST(ObjectAbi, typeNeedsMemoryReturnSysVCompleteRecordsOnly) {
    EXPECT_FALSE(typeNeedsMemoryReturn(type::signedInteger()));
    EXPECT_FALSE(typeNeedsMemoryReturn(type::signedLong()));
    auto small = type::structure({ { "a", type::signedLong() } });
    EXPECT_FALSE(typeNeedsMemoryReturn(small));
    auto twoWord = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
    });
    EXPECT_EQ(twoWord.getSize(), REGISTER_RETURN_MAX_BYTES);
    EXPECT_FALSE(typeNeedsMemoryReturn(twoWord));
    auto large = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
            { "c", type::signedLong() },
    });
    EXPECT_GT(large.getSize(), REGISTER_RETURN_MAX_BYTES);
    EXPECT_TRUE(typeNeedsMemoryReturn(large));
    auto bigArr = type::array(type::signedCharacter(), 24);
    EXPECT_TRUE(bigArr.isAggregate());
    EXPECT_FALSE(typeNeedsMemoryReturn(bigArr));
}

TEST(ObjectAbi, incompleteRecordIsNotSret) {
    type::Type inc = type::incompleteRecord();
    EXPECT_TRUE(inc.isRecord());
    EXPECT_TRUE(inc.isIncompleteRecord());
    EXPECT_FALSE(inc.isCompleteRecord());
    EXPECT_FALSE(typeNeedsMemoryReturn(inc));
}

TEST(ObjectAbi, largeUnionNeedsSret) {
    type::Type u = type::unionType({
            { "small", type::signedInteger() },
            { "big", type::array(type::signedLong(), 3) },
    });
    EXPECT_TRUE(u.isRecord());
    EXPECT_EQ(u.getSize(), 24);
    EXPECT_TRUE(typeNeedsMemoryReturn(u));
}

TEST(ObjectAbi, constants) {
    EXPECT_EQ(MACHINE_WORD_SIZE, 8);
    EXPECT_EQ(STACK_ALIGNMENT, 16);
    EXPECT_EQ(REGISTER_RETURN_MAX_BYTES, 16);
    EXPECT_STREQ(SRET_SYMBOL_NAME, "__sret");
}

} // namespace
