#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/types/NumericType.h"
#include "ast/types/Void.h"
#include "ast/types/Function.h"

using testing::Eq;

namespace ast {

TEST(BaseType, treatsIntegerAsInteger) {
    EXPECT_TRUE(BaseType::INTEGER.isInteger());
}

TEST(BaseType, treatsOtherTypesAsNonInteger) {
    EXPECT_FALSE(BaseType::CHARACTER.isInteger());
    EXPECT_FALSE(BaseType::FLOAT.isInteger());
    EXPECT_FALSE(BaseType::VOID.isInteger());
    //EXPECT_FALSE(BaseType::FUNCTION.isInteger());
}

TEST(BaseType, sameTypesAreEqual) {
    EXPECT_TRUE(BaseType::CHARACTER == BaseType::CHARACTER);
    EXPECT_TRUE(BaseType::INTEGER == BaseType::INTEGER);
    EXPECT_TRUE(BaseType::FLOAT == BaseType::FLOAT);
    EXPECT_TRUE(BaseType::VOID == BaseType::VOID);

    EXPECT_FALSE(BaseType::CHARACTER == BaseType::INTEGER);
    EXPECT_FALSE(BaseType::CHARACTER == BaseType::FLOAT);
    EXPECT_FALSE(BaseType::CHARACTER == BaseType::VOID);

    EXPECT_FALSE(BaseType::INTEGER == BaseType::CHARACTER);
    EXPECT_FALSE(BaseType::INTEGER == BaseType::FLOAT);
    EXPECT_FALSE(BaseType::INTEGER == BaseType::VOID);

    EXPECT_FALSE(BaseType::FLOAT == BaseType::CHARACTER);
    EXPECT_FALSE(BaseType::FLOAT == BaseType::INTEGER);
    EXPECT_FALSE(BaseType::FLOAT == BaseType::VOID);
}

TEST(BaseType, differentTypesAreNotEqual) {
    EXPECT_TRUE(BaseType::CHARACTER != BaseType::INTEGER);
    EXPECT_TRUE(BaseType::CHARACTER != BaseType::FLOAT);
    EXPECT_TRUE(BaseType::CHARACTER != BaseType::VOID);

    EXPECT_TRUE(BaseType::INTEGER != BaseType::CHARACTER);
    EXPECT_TRUE(BaseType::INTEGER != BaseType::FLOAT);
    EXPECT_TRUE(BaseType::INTEGER != BaseType::VOID);

    EXPECT_TRUE(BaseType::FLOAT != BaseType::CHARACTER);
    EXPECT_TRUE(BaseType::FLOAT != BaseType::INTEGER);
    EXPECT_TRUE(BaseType::FLOAT != BaseType::VOID);

    EXPECT_FALSE(BaseType::CHARACTER != BaseType::CHARACTER);
    EXPECT_FALSE(BaseType::INTEGER != BaseType::INTEGER);
    EXPECT_FALSE(BaseType::FLOAT != BaseType::FLOAT);
    EXPECT_FALSE(BaseType::VOID != BaseType::VOID);
}

TEST(BaseType, createsPolymorphicInstancesOfTypes) {
    auto pCharacter = BaseType::newCharacter();
    EXPECT_THAT(*pCharacter, Eq(BaseType::CHARACTER));

    auto pInteger = BaseType::newInteger();
    EXPECT_THAT(*pInteger, Eq(BaseType::INTEGER));

    auto pFloat = BaseType::newFloat();
    EXPECT_THAT(*pFloat, Eq(BaseType::FLOAT));

    auto pVoid = BaseType::newVoid();
    EXPECT_THAT(*pVoid, Eq(BaseType::VOID));
}

TEST(BaseType, promotesSameTypeToSameType) {
    EXPECT_TRUE(BaseType::CHARACTER.promote(BaseType::CHARACTER) == BaseType::CHARACTER);
    EXPECT_TRUE(BaseType::INTEGER.promote(BaseType::INTEGER) == BaseType::INTEGER);
    EXPECT_TRUE(BaseType::FLOAT.promote(BaseType::FLOAT) == BaseType::FLOAT);

    EXPECT_TRUE(BaseType::VOID.promote(BaseType::VOID) == BaseType::VOID);
}

TEST(BaseType, promotesCharacterToInteger) {
    EXPECT_TRUE(BaseType::CHARACTER.promote(BaseType::INTEGER) == BaseType::INTEGER);
    EXPECT_TRUE(BaseType::INTEGER.promote(BaseType::CHARACTER) == BaseType::INTEGER);
}

TEST(BaseType, promotesCharacterToFloat) {
    EXPECT_TRUE(BaseType::CHARACTER.promote(BaseType::FLOAT) == BaseType::FLOAT);
    EXPECT_TRUE(BaseType::FLOAT.promote(BaseType::CHARACTER) == BaseType::FLOAT);
}

TEST(BaseType, promotesIntegerToFloat) {
    EXPECT_TRUE(BaseType::INTEGER.promote(BaseType::FLOAT) == BaseType::FLOAT);
    EXPECT_TRUE(BaseType::FLOAT.promote(BaseType::INTEGER) == BaseType::FLOAT);
}

TEST(BaseType, throwsWhenPromotingVoidToNumeric) {
    EXPECT_THROW(BaseType::VOID.promote(BaseType::CHARACTER), TypePromotionException);
    EXPECT_THROW(BaseType::VOID.promote(BaseType::INTEGER), TypePromotionException);
    EXPECT_THROW(BaseType::VOID.promote(BaseType::FLOAT), TypePromotionException);
}

TEST(BaseType, throwsWhenPromotingNumericToVoid) {
    EXPECT_THROW(BaseType::CHARACTER.promote(BaseType::VOID), TypePromotionException);
    EXPECT_THROW(BaseType::INTEGER.promote(BaseType::VOID), TypePromotionException);
    EXPECT_THROW(BaseType::FLOAT.promote(BaseType::VOID), TypePromotionException);
}

TEST(BaseType, isPolymorphicallyCloneable) {
    std::unique_ptr<BaseType> character = BaseType::newCharacter();
    std::unique_ptr<BaseType> clonedCharacter = character->clone();
    EXPECT_TRUE(*character == *clonedCharacter);

    std::unique_ptr<BaseType> integer = BaseType::newInteger();
    std::unique_ptr<BaseType> clonedInteger = integer->clone();
    EXPECT_TRUE(*integer == *clonedInteger);

    std::unique_ptr<BaseType> floatType = BaseType::newFloat();
    std::unique_ptr<BaseType> clonedFloat = floatType->clone();
    EXPECT_TRUE(*floatType == *clonedFloat);

    std::unique_ptr<BaseType> voidType = BaseType::newVoid();
    std::unique_ptr<BaseType> clonedVoid = voidType->clone();
    EXPECT_TRUE(*voidType == *clonedVoid);
}

}
