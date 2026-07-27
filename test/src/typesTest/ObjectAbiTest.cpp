#include "gtest/gtest.h"

#include "types/ObjectAbi.h"
#include "types/Type.h"

namespace {

using type::object_abi::dataWords;
using type::object_abi::needsMemoryReturn;
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

TEST(ObjectAbi, sretPolicyAggregatesOnly) {
    EXPECT_FALSE(needsMemoryReturn(16));
    EXPECT_TRUE(needsMemoryReturn(17));

    type::Type i = type::signedInteger();
    EXPECT_FALSE(typeNeedsMemoryReturn(i));

    // 3 x long = 24 bytes aggregate
    type::Type s = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
            { "c", type::signedLong() },
    });
    EXPECT_TRUE(s.isAggregate());
    EXPECT_EQ(s.getSize(), 24);
    EXPECT_TRUE(typeNeedsMemoryReturn(s));
    EXPECT_FALSE(typeNeedsMemoryReturn(s, true)); // variadic skips sret
    EXPECT_TRUE(typeNeedsMemoryReturn(s, false));
}

TEST(ObjectAbi, smallStructNotSret) {
    type::Type s = type::structure({
            { "a", type::signedLong() },
            { "b", type::signedLong() },
    });
    EXPECT_EQ(s.getSize(), 16);
    EXPECT_FALSE(typeNeedsMemoryReturn(s));
}

} // namespace
