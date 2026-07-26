#include "Expression.h"

#include <cassert>
#include <stdexcept>

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

type::Type Expression::valueType(const symbols::AnnotationStore& store) const {
    if (const auto* r = store.value(this, symbols::ValueSlot::Result)) {
        return r->getType();
    }
    return expressionType();
}

bool Expression::hasDecayedArrayValue(const symbols::AnnotationStore& store) const {
    return isArrayObjectType() && hasResultSymbol(store) && valueType(store).isPointer();
}

void Expression::setTypeAndResult(symbols::AnnotationStore& store, symbols::ValueEntry result) {
    setType(result.getType());
    form = ValueForm::Scalar;
    store.setResult(this, std::move(result));
}

void Expression::setAggregateAddressResult(symbols::AnnotationStore& store,
        symbols::ValueEntry addressSymbol, const type::Type& aggregateType) {
    setType(aggregateType);
    form = ValueForm::AggregateAddress;
    store.setResult(this, std::move(addressSymbol));
}

void Expression::setFunctionDesignatorResult(symbols::AnnotationStore& store,
        symbols::ValueEntry addressSymbol) {
    setType(addressSymbol.getType());
    form = ValueForm::FunctionDesignator;
    lval = false;
    store.setResult(this, std::move(addressSymbol));
}

bool Expression::hasResultSymbol(const symbols::AnnotationStore& store) const {
    return store.hasResult(this);
}

symbols::ValueEntry* Expression::getResultSymbol(symbols::AnnotationStore& store) const {
    return store.result(this);
}

bool Expression::isLval() const {
    return lval;
}

symbols::ValueEntry* Expression::getLvalueSymbol(symbols::AnnotationStore& store) const {
    (void)store;
    return nullptr;
}

} // namespace ast
