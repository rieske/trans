#include "Expression.h"

#include <algorithm>
#include <stdexcept>

#include "../code_generator/ValueEntry.h"

namespace ast {

const std::string Expression::ID { "<expr>" };

void Expression::setTypeInfo(Type typeInfo) {
    this->typeInfo = std::unique_ptr<Type> { new Type { typeInfo } };
}

Type Expression::getTypeInfo() const {
    if (!typeInfo) {
        throw std::runtime_error { "typeInfo is null" };
    }
    return *typeInfo;
}

void Expression::setResultHolder(code_generator::ValueEntry* resultHolder) {
    this->resultHolder = std::move(resultHolder);
    setTypeInfo(this->resultHolder->getTypeInfo());
}

code_generator::ValueEntry* Expression::getResultHolder() const {
    if (!resultHolder) {
        throw std::runtime_error { "resultHolder is null" };
    }
    return resultHolder;
}

bool Expression::isLval() const {
    return lval;
}

}

