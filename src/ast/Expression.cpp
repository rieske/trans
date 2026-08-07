#include "Expression.h"
#include "symbols/AnnotationStore.h"

#include "symbols/AddressPlan.h"

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
    // Result is the sole post-SA value-type channel; no soft fallback to expressionType.
    throw std::runtime_error { "expression value type requires Result annotation" };
}

bool Expression::hasDecayedArrayValue(const symbols::AnnotationStore& store) const {
    return holdsAggregateAddress() && hasResult(store) && valueType(store).isPointer();
}

void Expression::setTypeAndResult(symbols::AnnotationStore& store, symbols::ValueEntry resultSymbol) {
    type = resultSymbol.getType();
    form = ValueForm::Scalar;
    store.setResult(this, std::move(resultSymbol));
}

void Expression::setAggregateAddressResult(symbols::AnnotationStore& store,
        symbols::ValueEntry addressSymbol, const type::Type& aggregateType) {
    type = aggregateType;
    form = ValueForm::AggregateAddress;
    store.setResult(this, std::move(addressSymbol));
}

void Expression::setFunctionDesignatorResult(symbols::AnnotationStore& store,
        symbols::ValueEntry addressSymbol, std::string functionName) {
    type = addressSymbol.getType();
    form = ValueForm::FunctionDesignator;
    lval = false;
    store.setResult(this, std::move(addressSymbol));
    store.setAddressPlan(this, symbols::AddressPlan {
            symbols::FunctionDesignatorPlan { std::move(functionName) } });
}

const std::string* Expression::functionDesignatorName(const symbols::AnnotationStore& store) const {
    if (!holdsFunctionDesignator()) {
        return nullptr;
    }
    const auto* plan = store.addressPlan(this);
    if (!plan) {
        return nullptr;
    }
    if (const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(plan)) {
        return &d->functionName;
    }
    return nullptr;
}

bool Expression::hasResult(const symbols::AnnotationStore& store) const {
    return store.hasResult(this);
}

symbols::ValueEntry* Expression::result(symbols::AnnotationStore& store) const {
    return store.result(this);
}

bool Expression::isLval() const {
    return lval;
}

symbols::ValueEntry* Expression::lvalueAnnotation(symbols::AnnotationStore& store) const {
    return store.value(this, symbols::ValueSlot::Lvalue);
}

} // namespace ast
