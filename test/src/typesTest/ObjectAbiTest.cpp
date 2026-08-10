#include "gtest/gtest.h"

#include "types/ObjectAbi.h"
#include "types/ObjectAbiType.h"
#include "types/Type.h"

namespace {

using type::object_abi::dataWords;
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
