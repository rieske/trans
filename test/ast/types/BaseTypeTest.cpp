#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/types/NumericType.h"
#include "ast/types/Void.h"
#include "ast/types/Function.h"

using testing::Eq;

namespace ast {

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

TEST(BaseType, convertsSameTypeToSameType) {
    EXPECT_TRUE(BaseType::CHARACTER.convertFrom(BaseType::CHARACTER) == BaseType::CHARACTER);
    EXPECT_TRUE(BaseType::INTEGER.convertFrom(BaseType::INTEGER) == BaseType::INTEGER);
    EXPECT_TRUE(BaseType::FLOAT.convertFrom(BaseType::FLOAT) == BaseType::FLOAT);

    EXPECT_TRUE(BaseType::VOID.convertFrom(BaseType::VOID) == BaseType::VOID);
}

TEST(BaseType, convertsCharacterToInteger) {
    EXPECT_TRUE(BaseType::CHARACTER.convertFrom(BaseType::INTEGER) == BaseType::INTEGER);
}

TEST(BaseType, convertsIntegerToCharacter) {
    EXPECT_TRUE(BaseType::INTEGER.convertFrom(BaseType::CHARACTER) == BaseType::CHARACTER);
}

TEST(BaseType, convertsCharacterToFloat) {
    EXPECT_TRUE(BaseType::CHARACTER.convertFrom(BaseType::FLOAT) == BaseType::FLOAT);
}

TEST(BaseType, convertsFloatToCharacter) {
    EXPECT_TRUE(BaseType::FLOAT.convertFrom(BaseType::CHARACTER) == BaseType::CHARACTER);
}

TEST(BaseType, convertsIntegerToFloat) {
    EXPECT_TRUE(BaseType::INTEGER.convertFrom(BaseType::FLOAT) == BaseType::FLOAT);
}

TEST(BaseType, convertsFloatToInteger) {
    EXPECT_TRUE(BaseType::FLOAT.convertFrom(BaseType::INTEGER) == BaseType::INTEGER);
}

TEST(BaseType, throwsWhenConvertingVoidToNumeric) {
    EXPECT_THROW(BaseType::VOID.convertFrom(BaseType::CHARACTER), TypeConversionException);
    EXPECT_THROW(BaseType::VOID.convertFrom(BaseType::INTEGER), TypeConversionException);
    EXPECT_THROW(BaseType::VOID.convertFrom(BaseType::FLOAT), TypeConversionException);
}

TEST(BaseType, throwsWhenConvertingNumericToVoid) {
    EXPECT_THROW(BaseType::CHARACTER.convertFrom(BaseType::VOID), TypeConversionException);
    EXPECT_THROW(BaseType::INTEGER.convertFrom(BaseType::VOID), TypeConversionException);
    EXPECT_THROW(BaseType::FLOAT.convertFrom(BaseType::VOID), TypeConversionException);
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

TEST(BaseType, canBeRepresentedAsString) {
    EXPECT_THAT(BaseType::CHARACTER.toString(), Eq("char"));
    EXPECT_THAT(BaseType::INTEGER.toString(), Eq("int"));
    EXPECT_THAT(BaseType::FLOAT.toString(), Eq("float"));
    EXPECT_THAT(BaseType::VOID.toString(), Eq("void"));
}

}
