#ifndef TYPECONVERTER_H_
#define TYPECONVERTER_H_

#include <stdexcept>

#include "../ast/types/Type.h"

namespace semantic_analyzer {

class TypeCheckException: public std::runtime_error {
};

class TypeConverter {
public:
    TypeConverter();
    virtual ~TypeConverter();

    ast::Type convertType(ast::Type type1, ast::Type type2);
};

} /* namespace semantic_analyzer */

#endif /* TYPECONVERTER_H_ */
