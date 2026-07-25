#include "Expression.h"

#include <cassert>

namespace ast {

void Expression::setType(const type::Type& type) {
    this->type = type;
}

type::Type Expression::expressionType() const {
    if (!type) {
        throw std::runtime_error { "expression type is not set" };
    }
    return *type;
}

type::Type Expression::valueType() const {
    if (resultSymbol) {
        return resultSymbol->getType();
    }
    return expressionType();
}

bool Expression::hasDecayedArrayValue() const {
    return isArrayObjectType() && hasResultSymbol() && valueType().isPointer();
}

void Expression::setTypeAndResult(semantic_analyzer::ValueEntry resultSymbol) {
    this->resultSymbol = std::make_unique<semantic_analyzer::ValueEntry>(std::move(resultSymbol));
    setType(this->resultSymbol->getType());
    form = ValueForm::Scalar;
    designatorName.clear();
}

void Expression::setAggregateAddressResult(semantic_analyzer::ValueEntry addressSymbol,
        const type::Type& aggregateType) {
    this->resultSymbol = std::make_unique<semantic_analyzer::ValueEntry>(std::move(addressSymbol));
    setType(aggregateType);
    form = ValueForm::AggregateAddress;
    designatorName.clear();
}

void Expression::setFunctionDesignatorResult(semantic_analyzer::ValueEntry addressSymbol,
        std::string designatorName) {
    this->resultSymbol = std::make_unique<semantic_analyzer::ValueEntry>(std::move(addressSymbol));
    setType(this->resultSymbol->getType());
    form = ValueForm::FunctionDesignator;
    this->designatorName = std::move(designatorName);
    lval = false;
}

bool Expression::hasResultSymbol() const {
    return resultSymbol != nullptr;
}

semantic_analyzer::ValueEntry* Expression::getResultSymbol() const {
    assert(resultSymbol);
    return resultSymbol.get();
}

const std::string& Expression::functionDesignatorName() const {
    assert(form == ValueForm::FunctionDesignator);
    return designatorName;
}

bool Expression::isLval() const {
    return lval;
}

semantic_analyzer::ValueEntry* Expression::getLvalueSymbol() const {
    return nullptr;
}

} // namespace ast
