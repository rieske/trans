#include "Expression.h"

#include <cassert>
#include <stdexcept>

#include "symbols/AnnotationStore.h"

namespace ast {

namespace {
constexpr const char* kDecay = "arrayDecaySource";
constexpr const char* kConv = "resultConversionTarget";
} // namespace

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
    if (hasResultSymbol()) {
        return getResultSymbol()->getType();
    }
    return expressionType();
}

void Expression::setTypedResult(symbols::ValueEntry resultSymbol) {
    type = resultSymbol.getType();
    symbols::AnnotationStore::current().setValue(
            this, symbols::ValueSlot::Result, std::move(resultSymbol));
}

void Expression::setResultSymbol(symbols::ValueEntry resultSymbol) {
    if (!type) {
        throw std::runtime_error {
                "setResultSymbol requires setType first (use setTypedResult for single-type)" };
    }
    symbols::AnnotationStore::current().setValue(
            this, symbols::ValueSlot::Result, std::move(resultSymbol));
}

bool Expression::hasResultSymbol() const {
    return symbols::AnnotationStore::hasCurrent()
            && symbols::AnnotationStore::current().hasValue(this, symbols::ValueSlot::Result);
}

symbols::ValueEntry* Expression::getResultSymbol() const {
    auto* r = symbols::AnnotationStore::current().value(this, symbols::ValueSlot::Result);
    assert(r);
    return r;
}

bool Expression::isLval() const {
    return lval;
}

symbols::ValueEntry* Expression::getLvalueSymbol() const {
    return nullptr;
}

void Expression::setArrayDecaySource(std::string arraySymbolName) {
    symbols::AnnotationStore::current().setString(this, kDecay, std::move(arraySymbolName));
}

const std::string* Expression::getArrayDecaySource() const {
    return symbols::AnnotationStore::current().string(this, kDecay);
}

void Expression::setResultConversionTarget(std::string targetSymbolName) {
    symbols::AnnotationStore::current().setString(this, kConv, std::move(targetSymbolName));
}

const std::string* Expression::getResultConversionTarget() const {
    return symbols::AnnotationStore::current().string(this, kConv);
}

} // namespace ast
