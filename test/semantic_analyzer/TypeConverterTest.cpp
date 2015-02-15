#include <gtest/gtest.h>
#include <semantic_analyzer/TypeConverter.h>

using ast::Type;

namespace semantic_analyzer {

TEST(TypeConverter, convertsTwoEqualTypesToTheSameType) {
    TypeConverter converter;

   /* const Type& integerType { Type::INTEGER };
    EXPECT_THAT(converter.convertType(integerType, integerType), Eq(Type::INTEGER));

    Type floatType { Type::FLOAT };
    EXPECT_THAT(converter.convertType(floatType, floatType), Eq(Type::FLOAT));*/
}

/*TEST(TypeConverter, convertsIntegerToFloat) {
    TypeConverter converter;

    TypeInfo integerType { BasicType::INTEGER };
    TypeInfo floatType { BasicType::FLOAT };
    EXPECT_THAT(converter.convertType(integerType, floatType), Eq(floatType));
    EXPECT_THAT(converter.convertType(floatType, integerType), Eq(floatType));
}*/

} /* namespace semantic_analyzer */
