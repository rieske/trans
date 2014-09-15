#include "semantic_analyzer/TypeConverter.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using ast::TypeInfo;
using ast::BasicType;

using testing::Eq;

namespace semantic_analyzer {

TEST(TypeConverter, convertsTwoEqualTypesToTheSameType) {
    TypeConverter converter;

    TypeInfo integerType { BasicType::INTEGER };
    EXPECT_THAT(converter.convertType(integerType, integerType), Eq(integerType));

    TypeInfo floatType { BasicType::FLOAT };
    EXPECT_THAT(converter.convertType(floatType, floatType), Eq(floatType));
}

/*TEST(TypeConverter, convertsIntegerToFloat) {
    TypeConverter converter;

    TypeInfo integerType { BasicType::INTEGER };
    TypeInfo floatType { BasicType::FLOAT };
    EXPECT_THAT(converter.convertType(integerType, floatType), Eq(floatType));
    EXPECT_THAT(converter.convertType(floatType, integerType), Eq(floatType));
}*/

} /* namespace semantic_analyzer */
