#include "ast/types/Function.h"
#include "ast/types/NumericType.h"
#include "ast/types/Void.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

using testing::Eq;

namespace ast {

TEST(Function, isCreatedWithArgumentTypes) {
    Type integerType { BaseType::newInteger() };
    Type voidPointer { BaseType::newVoid(), 1 };

    Function function { { integerType, voidPointer } };
}

TEST(Function, canBeCreatedWithoutArguments) {
    Function function { };
}

TEST(Function, isNotEqualToNonFunctionTypes) {
    Function function { };

    EXPECT_FALSE(function == BaseType::CHARACTER);
    EXPECT_FALSE(function == BaseType::INTEGER);
    EXPECT_FALSE(function == BaseType::FLOAT);
    EXPECT_FALSE(function == BaseType::VOID);
    EXPECT_FALSE(BaseType::CHARACTER == function);
    EXPECT_FALSE(BaseType::INTEGER == function);
    EXPECT_FALSE(BaseType::FLOAT == function);
    EXPECT_FALSE(BaseType::VOID == function);

    EXPECT_TRUE(function != BaseType::CHARACTER);
    EXPECT_TRUE(function != BaseType::INTEGER);
    EXPECT_TRUE(function != BaseType::FLOAT);
    EXPECT_TRUE(function != BaseType::VOID);
    EXPECT_TRUE(BaseType::CHARACTER != function);
    EXPECT_TRUE(BaseType::INTEGER != function);
    EXPECT_TRUE(BaseType::FLOAT != function);
    EXPECT_TRUE(BaseType::VOID != function);
}

TEST(Function, isEqualToFunctionWithSameArgumentList) {
    Function noargFunction { };
    Function anotherNoargFunction { };

    EXPECT_TRUE(noargFunction == noargFunction);
    EXPECT_TRUE(anotherNoargFunction == anotherNoargFunction);
    EXPECT_TRUE(noargFunction == anotherNoargFunction);
    EXPECT_TRUE(anotherNoargFunction == noargFunction);

    EXPECT_FALSE(noargFunction != noargFunction);
    EXPECT_FALSE(anotherNoargFunction != anotherNoargFunction);
    EXPECT_FALSE(noargFunction != anotherNoargFunction);
    EXPECT_FALSE(anotherNoargFunction != noargFunction);

    Type integerType { BaseType::newInteger() };
    Type voidPointer { BaseType::newVoid(), 1 };

    Function intVoidFunction { { integerType, voidPointer } };
    Function anotherIntVoidFunction { { integerType, voidPointer } };
    EXPECT_TRUE(intVoidFunction == intVoidFunction);
    EXPECT_TRUE(anotherIntVoidFunction == anotherIntVoidFunction);
    EXPECT_TRUE(intVoidFunction == anotherIntVoidFunction);
    EXPECT_TRUE(anotherIntVoidFunction == intVoidFunction);

    EXPECT_FALSE(intVoidFunction != intVoidFunction);
    EXPECT_FALSE(anotherIntVoidFunction != anotherIntVoidFunction);
    EXPECT_FALSE(intVoidFunction != anotherIntVoidFunction);
    EXPECT_FALSE(anotherIntVoidFunction != intVoidFunction);

    EXPECT_FALSE(noargFunction == intVoidFunction);
    EXPECT_FALSE(intVoidFunction == noargFunction);
    EXPECT_TRUE(intVoidFunction != noargFunction);
    EXPECT_TRUE(noargFunction != intVoidFunction);

    Function voidIntFunction { { voidPointer, integerType } };
    EXPECT_FALSE(voidIntFunction == intVoidFunction);
    EXPECT_FALSE(intVoidFunction == voidIntFunction);
    EXPECT_TRUE(voidIntFunction != intVoidFunction);
    EXPECT_TRUE(intVoidFunction != voidIntFunction);
}

TEST(Function, throwsWhenConvertingToNumericType) {
    Function function { };

    EXPECT_THROW(function.convertTo(BaseType::CHARACTER), TypeConversionException);
    EXPECT_THROW(function.convertTo(BaseType::INTEGER), TypeConversionException);
    EXPECT_THROW(function.convertTo(BaseType::FLOAT), TypeConversionException);
}

TEST(Function, throwsWhenConvertingToVoidType) {
    Function function { };

    EXPECT_THROW(function.convertTo(BaseType::VOID), TypeConversionException);
}

TEST(Function, throwsWhenConvertingFromNumericType) {
    Function function { };

    EXPECT_THROW(BaseType::CHARACTER.convertTo(function), TypeConversionException);
    EXPECT_THROW(BaseType::INTEGER.convertTo(function), TypeConversionException);
    EXPECT_THROW(BaseType::FLOAT.convertTo(function), TypeConversionException);
}

TEST(Function, throwsWhenConvertingFromVoidType) {
    Function function { };

    EXPECT_THROW(BaseType::VOID.convertTo(function), TypeConversionException);
}

TEST(Function, throwsWhenConvertingToDifferentFunctionType) {
    Type integerType { BaseType::newInteger() };
    Type voidPointer { BaseType::newVoid(), 1 };

    Function functionWithArguments { { integerType, voidPointer } };
    Function noargFunction { };

    EXPECT_THROW(noargFunction.convertTo(functionWithArguments), TypeConversionException);
    EXPECT_THROW(functionWithArguments.convertTo(noargFunction), TypeConversionException);
}

TEST(Function, convertsFunctionsWithSameArguments) {
    Type integerType { BaseType::newInteger() };
    Type voidPointer { BaseType::newVoid(), 1 };

    Function functionWithArguments { { integerType, voidPointer } };
    Function anotherFunctionWithArguments { { integerType, voidPointer } };
    EXPECT_THAT(functionWithArguments.convertTo(anotherFunctionWithArguments), Eq(functionWithArguments));
    EXPECT_THAT(functionWithArguments.convertTo(anotherFunctionWithArguments), Eq(anotherFunctionWithArguments));

    Function noargFunction { };
    Function anotherNoargFunction { };
    EXPECT_THAT(noargFunction.convertTo(anotherNoargFunction), Eq(noargFunction));
    EXPECT_THAT(noargFunction.convertTo(anotherNoargFunction), Eq(anotherNoargFunction));
}

TEST(Function, isPolymorphicallyCloneable) {
    std::unique_ptr<BaseType> noargFunction { new Function { } };
    std::unique_ptr<BaseType> clonedFunction = noargFunction->clone();
    EXPECT_TRUE(*clonedFunction == *noargFunction);

    Type integerType { BaseType::newInteger() };
    Type voidPointer { BaseType::newVoid(), 1 };
    std::unique_ptr<BaseType> functionWithArguments { new Function { { integerType, voidPointer } } };
    std::unique_ptr<BaseType> clonedFunctionWithArguments = functionWithArguments->clone();
    EXPECT_TRUE(*clonedFunctionWithArguments == *functionWithArguments);
}

}
