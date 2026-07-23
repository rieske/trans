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

bool Expression::hasResultSymbol() const {
    return resultSymbol != nullptr;
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

void Expression::setArrayDecaySource(std::string arraySymbolName) {
    arrayDecaySource = std::move(arraySymbolName);
}

const std::string* Expression::getArrayDecaySource() const {
    return arrayDecaySource ? &*arrayDecaySource : nullptr;
}

void Expression::setResultConversionTarget(std::string targetSymbolName) {
    resultConversionTarget = std::move(targetSymbolName);
}

const std::string* Expression::getResultConversionTarget() const {
    return resultConversionTarget ? &*resultConversionTarget : nullptr;
}

} // namespace ast
