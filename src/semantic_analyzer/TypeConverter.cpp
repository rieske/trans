#include "TypeConverter.h"

namespace semantic_analyzer {

TypeConverter::TypeConverter() {
}

TypeConverter::~TypeConverter() {
}

ast::Type TypeConverter::convertType(ast::Type typeFrom, ast::Type typeTo) {
    if (typeFrom == typeTo) {
        return typeTo;
    }
    return typeTo;
}

} /* namespace semantic_analyzer */
