#include "gtest/gtest.h"

#include "types/ObjectAbi.h"
#include "types/ObjectAbiType.h"
#include "types/Type.h"

namespace {

using type::object_abi::MACHINE_WORD_SIZE;
using type::object_abi::STACK_ALIGNMENT;
using type::object_abi::alignUp;
using type::object_abi::alignWordIndex;
using type::object_abi::dataWords;
using type::object_abi::frameLayout;
using type::object_abi::takeAlignedWords;
using type::object_abi::typeNeedsMemoryReturn;
using type::object_abi::valueWords;
using type::object_abi::wordByteOffset;
using type::object_abi::wordIndexAt;

TEST(ObjectAbi, valueWordsAtLeastOne) {
    EXPECT_EQ(valueWords(0), 1);
    EXPECT_EQ(valueWords(-1), 1);
    EXPECT_EQ(valueWords(1), 1);
    EXPECT_EQ(valueWords(8), 1);
    EXPECT_EQ(valueWords(9), 2);
    EXPECT_EQ(valueWords(16), 2);
    EXPECT_EQ(valueWords(17), 3);
}

TEST(ObjectAbi, dataWordsZeroWhenEmpty) {
    EXPECT_EQ(dataWords(0), 0);
    EXPECT_EQ(dataWords(-3), 0);
    EXPECT_EQ(dataWords(8), 1);
    EXPECT_EQ(dataWords(24), 3);
}

TEST(ObjectAbi, wordIndexHelpers) {
    EXPECT_EQ(wordByteOffset(0), 0);
    EXPECT_EQ(wordByteOffset(2), 16);
    EXPECT_EQ(wordIndexAt(0), 0);
    EXPECT_EQ(wordIndexAt(8), 1);
    EXPECT_EQ(wordIndexAt(15), 1);
}

TEST(ObjectAbi, alignUpAndWordIndex) {
    EXPECT_EQ(alignUp(0, 16), 0);
    EXPECT_EQ(alignUp(8, 16), 16);
    EXPECT_EQ(alignUp(16, 16), 16);
    EXPECT_EQ(alignUp(24, 16), 32);
    EXPECT_EQ(alignUp(7, 1), 7);
    EXPECT_EQ(alignWordIndex(0, 16), 0);
    EXPECT_EQ(alignWordIndex(1, 16), 2);
    EXPECT_EQ(alignWordIndex(2, 16), 2);
    EXPECT_EQ(alignWordIndex(3, 8), 3);
}

TEST(ObjectAbi, takeAlignedWordsPadsThenAdvances) {
    int next = 1;
    EXPECT_EQ(takeAlignedWords(next, 16, 2), 2);
    EXPECT_EQ(next, 4);
    EXPECT_EQ(takeAlignedWords(next, 8, 1), 4);
    EXPECT_EQ(next, 5);
}

TEST(ObjectAbi, takeAlignedWordsIdentityWhenAlreadyAligned) {
    int next = 0;
    EXPECT_EQ(takeAlignedWords(next, 16, 2), 0);
    EXPECT_EQ(next, 2);
}

TEST(ObjectAbi, frameLayoutHomeExcludesCallPad) {
    const int fiveCalleeSaved = 5 * MACHINE_WORD_SIZE;
    const auto empty = frameLayout(0, fiveCalleeSaved);
    EXPECT_EQ(empty.homeBytes, 0);
    EXPECT_EQ(empty.subBytes, MACHINE_WORD_SIZE);

    const auto twoWords = frameLayout(2, fiveCalleeSaved);
    EXPECT_EQ(twoWords.homeBytes, STACK_ALIGNMENT);
    EXPECT_EQ(twoWords.subBytes, STACK_ALIGNMENT + MACHINE_WORD_SIZE);

    const auto oddWord = frameLayout(1, fiveCalleeSaved);
    EXPECT_EQ(oddWord.homeBytes, STACK_ALIGNMENT);
    EXPECT_EQ(oddWord.subBytes, STACK_ALIGNMENT + MACHINE_WORD_SIZE);
}

TEST(ObjectAbi, frameLayoutNoCallPadWhenAlreadyAligned) {
    const int fourCalleeSaved = 4 * MACHINE_WORD_SIZE;
    const auto empty = frameLayout(0, fourCalleeSaved);
    EXPECT_EQ(empty.homeBytes, 0);
    EXPECT_EQ(empty.subBytes, 0);

    const auto oneWord = frameLayout(1, fourCalleeSaved);
    EXPECT_EQ(oneWord.homeBytes, STACK_ALIGNMENT);
    EXPECT_EQ(oneWord.subBytes, STACK_ALIGNMENT);

    const auto noSaved = frameLayout(1, 0);
    EXPECT_EQ(noSaved.homeBytes, STACK_ALIGNMENT);
    EXPECT_EQ(noSaved.subBytes, STACK_ALIGNMENT);
}

TEST(ObjectAbi, sretPolicyRecordsOnly) {
    type::Type i = type::signedInteger();
    EXPECT_FALSE(typeNeedsMemoryReturn(i));

    type::Type s = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
            { "c", type::signedLong() },
    });
    EXPECT_TRUE(s.isRecord());
    EXPECT_EQ(s.getSize(), 24);
    EXPECT_TRUE(typeNeedsMemoryReturn(s));
}

TEST(ObjectAbi, smallStructNotSret) {
    type::Type s = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
    });
    EXPECT_EQ(s.getSize(), 16);
    EXPECT_FALSE(typeNeedsMemoryReturn(s));
}

TEST(ObjectAbi, largeArrayIsNotSret) {
    // Arrays are aggregates but not memory-returned (C cannot return arrays by value).
    type::Type arr = type::array(type::signedLong(), 4); // 32 bytes
    EXPECT_TRUE(arr.isAggregate());
    EXPECT_FALSE(typeNeedsMemoryReturn(arr));
}

TEST(ObjectAbi, largeUnionNeedsSret) {
    // Union size is max member stride; use a 24-byte arm.
    type::Type u = type::unionType({
            { "small", type::signedInteger() },
            { "big", type::array(type::signedLong(), 3) },
    });
    EXPECT_TRUE(u.isRecord());
    EXPECT_EQ(u.getSize(), 24);
    EXPECT_TRUE(typeNeedsMemoryReturn(u));
}

TEST(ObjectAbi, incompleteRecordIsNotSret) {
    type::Type inc = type::incompleteRecord();
    EXPECT_TRUE(inc.isRecord());
    EXPECT_FALSE(inc.isCompleteRecord());
    EXPECT_FALSE(typeNeedsMemoryReturn(inc));
}

} // namespace
