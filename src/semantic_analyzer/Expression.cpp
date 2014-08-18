#include "Expression.h"

#include <algorithm>

#include "../code_generator/symbol_table.h"

namespace semantic_analyzer {

const std::string Expression::ID { "<expr>" };

BasicType Expression::getBasicType() const {
    return basicType;
}

std::string Expression::getExtendedType() const {
    return extended_type;
}

void Expression::setResultHolder(SymbolEntry* resultHolder) {
    this->resultHolder = std::move(resultHolder);
}

SymbolEntry* Expression::getResultHolder() const {
    return resultHolder;
}

bool Expression::isLval() const {
    return lval;
}

}

