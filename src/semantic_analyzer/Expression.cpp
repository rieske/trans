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

std::string Expression::getValue() const {
    return value;
}

void Expression::saveExpressionAttributes(const Expression& expression) {
    value = expression.getValue();
    basicType = expression.getBasicType();
    extended_type = expression.getExtendedType();
}

void Expression::setResultHolder(SymbolEntry* resultHolder) {
    this->resultHolder = std::move(resultHolder);
}

SymbolEntry* Expression::getResultHolder() const {
    return resultHolder;
}

}

