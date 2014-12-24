#include "Expression.h"

#include <stdexcept>


namespace ast {

const std::string Expression::ID { "<expr>" };

void Expression::setType(Type typeInfo) {
    this->type = std::unique_ptr<Type> { new Type { typeInfo } };
}

Type Expression::getType() const {
    if (!type) {
        throw std::runtime_error { "type is null" };
    }
    return *type;
}

void Expression::setResultHolder(code_generator::ValueEntry resultHolder) {
    this->resultHolder = std::make_unique<code_generator::ValueEntry>(resultHolder);
    setType(this->resultHolder->getType());
}

code_generator::ValueEntry* Expression::getResultHolder() const {
    if (!resultHolder) {
        throw std::runtime_error { "resultHolder is null" };
    }
    return resultHolder.get();
}

bool Expression::isLval() const {
    return lval;
}

}

