#include "Expression.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace semantic_analyzer {

const std::string Expression::ID { "<expr>" };

void Expression::setTypeInfo(TypeInfo typeInfo) {
    this->typeInfo = std::unique_ptr<TypeInfo> { new TypeInfo { typeInfo } };
}

TypeInfo Expression::getTypeInfo() const {
    if (!typeInfo) {
        throw std::runtime_error { "typeInfo is null" };
    }
    return *typeInfo;
}

BasicType Expression::getBasicType() const {
    if (!typeInfo) {
        throw std::runtime_error { "basic type is null" };
    }
    return typeInfo->getBasicType();
}

std::string Expression::getExtendedType() const {
    if (!typeInfo) {
        throw std::runtime_error { "extended type is null" };
    }
    return typeInfo->getExtendedType();
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

