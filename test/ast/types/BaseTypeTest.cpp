#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/types/BaseType.h"

using testing::Eq;

namespace ast {

TEST(BaseType, treatsIntegerAsInteger) {
    EXPECT_TRUE(BaseType::INTEGER.isInteger());
}

TEST(BaseType, treatsOtherTypesAsNonInteger) {
    EXPECT_FALSE(BaseType::CHARACTER.isInteger());
    EXPECT_FALSE(BaseType::FLOAT.isInteger());
}

TEST(BaseType, sameTypesAreEqual) {
    EXPECT_TRUE(BaseType::CHARACTER == BaseType::CHARACTER);
    EXPECT_TRUE(BaseType::INTEGER == BaseType::INTEGER);
    EXPECT_TRUE(BaseType::FLOAT == BaseType::FLOAT);

    EXPECT_FALSE(BaseType::CHARACTER == BaseType::INTEGER);
    EXPECT_FALSE(BaseType::CHARACTER == BaseType::FLOAT);

    EXPECT_FALSE(BaseType::INTEGER == BaseType::CHARACTER);
    EXPECT_FALSE(BaseType::INTEGER == BaseType::FLOAT);

    EXPECT_FALSE(BaseType::FLOAT == BaseType::CHARACTER);
    EXPECT_FALSE(BaseType::FLOAT == BaseType::INTEGER);
}

TEST(BaseType, differentTypesAsNotEqual) {
    EXPECT_TRUE(BaseType::CHARACTER != BaseType::INTEGER);
    EXPECT_TRUE(BaseType::CHARACTER != BaseType::FLOAT);

    EXPECT_TRUE(BaseType::INTEGER != BaseType::CHARACTER);
    EXPECT_TRUE(BaseType::INTEGER != BaseType::FLOAT);

    EXPECT_TRUE(BaseType::FLOAT != BaseType::CHARACTER);
    EXPECT_TRUE(BaseType::FLOAT != BaseType::INTEGER);

    EXPECT_FALSE(BaseType::CHARACTER != BaseType::CHARACTER);
    EXPECT_FALSE(BaseType::INTEGER != BaseType::INTEGER);
    EXPECT_FALSE(BaseType::FLOAT != BaseType::FLOAT);
}

TEST(BaseType, promotesCharacterToInteger) {
    EXPECT_TRUE(BaseType::promote(BaseType::CHARACTER, BaseType::INTEGER) == BaseType::INTEGER);
    EXPECT_TRUE(BaseType::promote(BaseType::INTEGER, BaseType::CHARACTER) == BaseType::INTEGER);
}

TEST(BaseType, promotesCharacterToFloat) {
    EXPECT_TRUE(BaseType::promote(BaseType::CHARACTER, BaseType::FLOAT) == BaseType::FLOAT);
    EXPECT_TRUE(BaseType::promote(BaseType::FLOAT, BaseType::CHARACTER) == BaseType::FLOAT);
}

TEST(BaseType, promotesIntegerToFloat) {
    EXPECT_TRUE(BaseType::promote(BaseType::INTEGER, BaseType::FLOAT) == BaseType::FLOAT);
    EXPECT_TRUE(BaseType::promote(BaseType::FLOAT, BaseType::INTEGER) == BaseType::FLOAT);
}

}
