#include "Expression.h"

#include <stdexcept>


namespace ast {

const std::string Expression::ID { "<exp>" };

void Expression::setType(Type type) {
    this->type = std::unique_ptr<Type> { new Type { type } };
}

Type Expression::getType() const {
    if (!type) {
        throw std::runtime_error { "type is null" };
    }
    return *type;
}

void Expression::setResultSymbol(code_generator::ValueEntry resultSymbol) {
    this->resultSymbol = std::make_unique<code_generator::ValueEntry>(resultSymbol);
    setType(this->resultSymbol->getType());
}

code_generator::ValueEntry* Expression::getResultSymbol() const {
    if (!resultSymbol) {
        throw std::runtime_error { "resultSymbol is null" };
    }
    return resultSymbol.get();
}

bool Expression::isLval() const {
    return lval;
}

}

