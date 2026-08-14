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
        symbols::ValueEntry addressSymbol, const type::Type& functionType) {
    setType(functionType);
    form = ValueForm::FunctionDesignator;
    lval = false;
    store.setResult(this, std::move(addressSymbol));
}

void Expression::takeValueFrom(Expression& src, symbols::AnnotationStore& store) {
    assert(src.hasResultSymbol(store));
    if (src.holdsAggregateAddress()) {
        setAggregateAddressResult(store, *src.getResultSymbol(store), src.expressionType());
    } else if (src.holdsFunctionDesignator()) {
        setFunctionDesignatorResult(store, *src.getResultSymbol(store), src.expressionType());
    } else {
        setTypeAndResult(store, *src.getResultSymbol(store));
    }
    if (auto* addr = src.getLvalueSymbol(store)) {
        setLvalueSymbol(store, *addr);
    }
    if (const auto* plan = store.addressPlan(&src)) {
        store.setAddressPlan(this, *plan);
    }
    lval = src.isLval();
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

void Expression::setLvalueSymbol(symbols::AnnotationStore& store, symbols::ValueEntry address) {
    store.setLvalue(this, std::move(address));
}

symbols::ValueEntry* Expression::getLvalueSymbol(symbols::AnnotationStore& store) const {
    return store.lvalue(this);
}

symbols::ValueEntry* Expression::addressSymbol(symbols::AnnotationStore& store) const {
    if (!valueType(store).isPointer()) {
        if (auto* lv = getLvalueSymbol(store); lv && lv->getType().isPointer()) {
            return lv;
        }
    }
    return getResultSymbol(store);
}

} // namespace ast
