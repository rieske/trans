#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/types/Type.h"
#include "ast/types/NumericType.h"
#include "ast/types/Void.h"
#include "ast/types/Function.h"

#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>
#include <stdexcept>

using testing::Eq;

namespace ast {

class TypeTest: public testing::Test {
protected:

    virtual void SetUp() {
        numericTypeCreators.push_back(&BaseType::newCharacter);
        numericTypeCreators.push_back(&BaseType::newInteger);
        numericTypeCreators.push_back(&BaseType::newFloat);

        typeCreators.push_back(&BaseType::newVoid);
        typeCreators.insert(typeCreators.end(), numericTypeCreators.begin(), numericTypeCreators.end());
    }

    std::vector<std::function<std::unique_ptr<BaseType>()>> numericTypeCreators;
    std::vector<std::function<std::unique_ptr<BaseType>()>> typeCreators;
};

TEST_F(TypeTest, throwsForNegativeDereferenceCount) {
    for (auto createBaseType : typeCreators) {
        EXPECT_THROW(Type(createBaseType(), -1), std::invalid_argument);
    }
}

TEST_F(TypeTest, plainTypeEqualsAnotherSamePlainType) {
    for (auto createBaseType : typeCreators) {
        Type plainType { createBaseType() };
        Type anotherPlainType { createBaseType() };

        EXPECT_TRUE(plainType == plainType);
        EXPECT_TRUE(plainType == anotherPlainType);
        EXPECT_TRUE(anotherPlainType == plainType);
        EXPECT_FALSE(plainType != anotherPlainType);
        EXPECT_FALSE(plainType != plainType);
        EXPECT_FALSE(anotherPlainType != plainType);
    }
}

TEST_F(TypeTest, pointerToTypeEqualsAnotherPointerToSameType) {
    for (auto createBaseType : typeCreators) {
        Type pointerToType { createBaseType(), 1 };
        Type anotherPointerToType { createBaseType(), 1 };

        EXPECT_TRUE(pointerToType == pointerToType);
        EXPECT_TRUE(pointerToType == anotherPointerToType);
        EXPECT_TRUE(anotherPointerToType == pointerToType);
        EXPECT_FALSE(pointerToType != anotherPointerToType);
        EXPECT_FALSE(pointerToType != pointerToType);
        EXPECT_FALSE(anotherPointerToType != pointerToType);
    }
}

TEST_F(TypeTest, recognizesPointers) {
    for (auto createBaseType : typeCreators) {
        Type plainType { createBaseType() };
        EXPECT_FALSE(plainType.isPointer());
    }

    for (auto createBaseType : typeCreators) {
        Type pointerType { createBaseType(), 1 };
        EXPECT_TRUE(pointerType.isPointer());
    }
}

TEST_F(TypeTest, recognizesPlainVoidType) {
    for (auto createNumericType : numericTypeCreators) {
        Type numericType { createNumericType() };
        EXPECT_FALSE(numericType.isPlainVoid());

        Type numericTypePointer { createNumericType(), 1 };
        EXPECT_FALSE(numericTypePointer.isPlainVoid());
    }

    Type voidPointer { BaseType::newVoid(), 1 };
    EXPECT_FALSE(voidPointer.isPlainVoid());

    Type plainVoid { BaseType::newVoid() };
    EXPECT_TRUE(plainVoid.isPlainVoid());
}

TEST_F(TypeTest, recognizesPlainIntegerType) {
    for (auto createType : typeCreators) {
        if (BaseType::INTEGER != *createType()) {
            Type nonIntegerType { createType() };
            EXPECT_FALSE(nonIntegerType.isPlainInteger());
        } else {
            Type integerType { createType() };
            EXPECT_TRUE(integerType.isPlainInteger());
        }
    }

    for (auto createType : typeCreators) {
        Type pointerType { createType(), 1 };
        EXPECT_FALSE(pointerType.isPlainInteger());
    }
}

TEST_F(TypeTest, typesAreCopyable) {
    for (auto createType : typeCreators) {
        Type plainType { createType() };
        Type copyOfPlainType { plainType };
        EXPECT_THAT(plainType, Eq(copyOfPlainType));

        Type pointerType { createType(), 1 };
        Type copyOfPointerType { pointerType };
        EXPECT_THAT(pointerType, Eq(copyOfPointerType));
    }
}

TEST_F(TypeTest, typesAreCopyAssignable) {
    for (auto createType : typeCreators) {
        Type plainType { createType() };
        Type copyOfPlainType { createType(), 1 };
        copyOfPlainType = plainType;
        EXPECT_THAT(plainType, Eq(copyOfPlainType));

        Type pointerType { createType(), 1 };
        Type copyOfPointerType { createType() };
        copyOfPointerType = pointerType;
        EXPECT_THAT(pointerType, Eq(copyOfPointerType));
    }
}

TEST_F(TypeTest, getsAddressOfType) {
    for (auto createType : typeCreators) {
        Type plainType { createType() };
        EXPECT_FALSE(plainType.isPointer());
        Type addressOfType { plainType.getAddressType() };
        EXPECT_TRUE(addressOfType.isPointer());
    }

    for (auto createType : typeCreators) {
        Type pointerType { createType(), 5 };
        EXPECT_TRUE(pointerType.isPointer());
        Type addressOfType { pointerType.getAddressType() };
        EXPECT_TRUE(addressOfType.isPointer());
    }
}

TEST_F(TypeTest, dereferencesPointerTypes) {
    for (auto createType : typeCreators) {
        Type pointerType { createType(), 1 };
        EXPECT_TRUE(pointerType.isPointer());
        Type plainType { pointerType.getTypePointedTo() };
        EXPECT_FALSE(plainType.isPointer());
    }
}

TEST_F(TypeTest, throwsWhenDereferencingPlainType) {
    for (auto createType : typeCreators) {
        Type plainType { createType() };
        EXPECT_FALSE(plainType.isPointer());
        EXPECT_THROW(plainType.getTypePointedTo(), std::invalid_argument);
    }
}

TEST_F(TypeTest, canBeRepresentedAsString) {
    for (auto createType : typeCreators) {
        Type plainType { createType() };
        EXPECT_THAT(plainType.toString(), Eq(createType()->toString()));
    }

    for (auto createType : typeCreators) {
        Type pointerType { createType(), 1 };
        EXPECT_THAT(pointerType.toString(), Eq(createType()->toString() + "*"));
    }

    for (auto createType : typeCreators) {
        Type doublePointer { createType(), 2 };
        EXPECT_THAT(doublePointer.toString(), Eq(createType()->toString() + "**"));
    }
}

TEST_F(TypeTest, canBeConvertedToMatchingPointerAndBaseType) {
    for (auto createType : typeCreators) {
        Type plainType { createType() };
        Type anotherPlainType { createType() };
        EXPECT_TRUE(plainType.canConvertTo(anotherPlainType));
        EXPECT_TRUE(anotherPlainType.canConvertTo(plainType));
        EXPECT_TRUE(plainType.canConvertTo(plainType));
    }

    for (auto createType : typeCreators) {
        Type pointerType { createType(), 1 };
        Type anotherPointerType { createType(), 1 };
        EXPECT_TRUE(pointerType.canConvertTo(anotherPointerType));
        EXPECT_TRUE(anotherPointerType.canConvertTo(pointerType));
        EXPECT_TRUE(pointerType.canConvertTo(pointerType));
    }

    for (auto createType : typeCreators) {
        Type doublePointer { createType(), 2 };
        Type anotherDoublePointer { createType(), 2 };
        EXPECT_TRUE(doublePointer.canConvertTo(anotherDoublePointer));
        EXPECT_TRUE(anotherDoublePointer.canConvertTo(doublePointer));
        EXPECT_TRUE(doublePointer.canConvertTo(doublePointer));
    }
}

TEST_F(TypeTest, canNotBeConvertedToNonMatchintPointerType) {
    for (auto createType : typeCreators) {
        Type plainType { createType() };
        Type pointerType { createType(), 1};
        EXPECT_FALSE(plainType.canConvertTo(pointerType));
        EXPECT_FALSE(pointerType.canConvertTo(plainType));
    }

    for (auto createType : typeCreators) {
        Type pointerType { createType(), 1 };
        Type anotherPointerType { createType(), 2 };
        EXPECT_FALSE(pointerType.canConvertTo(anotherPointerType));
        EXPECT_FALSE(anotherPointerType.canConvertTo(pointerType));
    }
}

}
