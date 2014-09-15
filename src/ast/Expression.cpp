#include "Expression.h"

#include <algorithm>
#include <stdexcept>

#include "../code_generator/symbol_entry.h"

namespace ast {

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

void Expression::setResultHolder(SymbolEntry* resultHolder) {
    this->resultHolder = std::move(resultHolder);
    setTypeInfo(this->resultHolder->getTypeInfo());
}

SymbolEntry* Expression::getResultHolder() const {
    if (!resultHolder) {
        throw std::runtime_error { "resultHolder is null" };
    }
    return resultHolder;
}

bool Expression::isLval() const {
    return lval;
}

}

