#ifndef TYPECONVERTER_H_
#define TYPECONVERTER_H_

#include <stdexcept>

#include "../ast/TypeInfo.h"

namespace semantic_analyzer {

class TypeCheckException: public std::runtime_error {
};

class TypeConverter {
public:
    TypeConverter();
    virtual ~TypeConverter();

    ast::TypeInfo convertType(ast::TypeInfo type1, ast::TypeInfo type2);
};

} /* namespace semantic_analyzer */

#endif /* TYPECONVERTER_H_ */
