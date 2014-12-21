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

TEST(Function, canNotBeConvertedToNumericType) {
    Function function { };

    EXPECT_FALSE(function.canConvertTo(BaseType::CHARACTER));
    EXPECT_FALSE(function.canConvertTo(BaseType::INTEGER));
    EXPECT_FALSE(function.canConvertTo(BaseType::FLOAT));
}

TEST(Function, canNotBeConvertedToVoidType) {
    Function function { };

    EXPECT_FALSE(function.canConvertTo(BaseType::VOID));
}

TEST(Function, canNotBeConvertedFromNumericType) {
    Function function { };

    EXPECT_FALSE(BaseType::CHARACTER.canConvertTo(function));
    EXPECT_FALSE(BaseType::INTEGER.canConvertTo(function));
    EXPECT_FALSE(BaseType::FLOAT.canConvertTo(function));
}

TEST(Function, canNotBeConvertedFromVoidType) {
    Function function { };

    EXPECT_FALSE(BaseType::VOID.canConvertTo(function));
}

TEST(Function, canNotBeConvertedToDifferentFunctionType) {
    Type integerType { BaseType::newInteger() };
    Type voidPointer { BaseType::newVoid(), 1 };

    Function functionWithArguments { { integerType, voidPointer } };
    Function noargFunction { };

    EXPECT_FALSE(noargFunction.canConvertTo(functionWithArguments));
    EXPECT_FALSE(functionWithArguments.canConvertTo(noargFunction));
}

TEST(Function, functionsWithSameArgumentsCanBeConvertedToOneAnother) {
    Type integerType { BaseType::newInteger() };
    Type voidPointer { BaseType::newVoid(), 1 };

    Function functionWithArguments { { integerType, voidPointer } };
    Function anotherFunctionWithArguments { { integerType, voidPointer } };
    EXPECT_TRUE(functionWithArguments.canConvertTo(anotherFunctionWithArguments));
    EXPECT_TRUE(functionWithArguments.canConvertTo(anotherFunctionWithArguments));

    Function noargFunction { };
    Function anotherNoargFunction { };
    EXPECT_TRUE(noargFunction.canConvertTo(anotherNoargFunction));
    EXPECT_TRUE(noargFunction.canConvertTo(anotherNoargFunction));
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
