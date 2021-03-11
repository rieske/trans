#include "Expression.h"

#include <cassert>

namespace ast {

void Expression::setType(const type::Type& type) {
    this->type = type;
}

type::Type Expression::getType() const {
    if (!type) {
        throw std::runtime_error { "type is not set" };
    }
    return *type;
}

void Expression::setResultSymbol(semantic_analyzer::ValueEntry resultSymbol) {
    this->resultSymbol = std::make_unique<semantic_analyzer::ValueEntry>(resultSymbol);
    setType(this->resultSymbol->getType());
}

semantic_analyzer::ValueEntry* Expression::getResultSymbol() const {
    assert(resultSymbol);
    return resultSymbol.get();
}

bool Expression::isLval() const {
    return lval;
}

semantic_analyzer::ValueEntry* Expression::getLvalueSymbol() const {
    return nullptr;
}

} // namespace ast

