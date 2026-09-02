#ifndef AGGREGATE_INIT_ERROR_H_
#define AGGREGATE_INIT_ERROR_H_

#include "types/Type.h"

namespace semantic_analyzer {

inline const char* unsizedArrayInitError(const type::Type& type) {
    if (type.isIncompleteArray()) {
        return "initialization of a flexible array member";
    }
    return "array brace initializers for incomplete arrays are not implemented";
}

} // namespace semantic_analyzer

#endif
