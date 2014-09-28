#include "BaseType.h"

#include <algorithm>

#include "Function.h"
#include "NumericType.h"
#include "Void.h"

namespace ast {

const NumericType BaseType::CHARACTER { NumericType::createCharacter() };
const NumericType BaseType::INTEGER { NumericType::createInteger() };
const NumericType BaseType::FLOAT { NumericType::createFloat() };
const Void BaseType::VOID { };
//const Function BaseType::FUNCTION { };

std::unique_ptr<BaseType> BaseType::newCharacter() {
    return std::unique_ptr<BaseType> { new NumericType { CHARACTER } };
}

std::unique_ptr<BaseType> BaseType::newInteger() {
    return std::unique_ptr<BaseType> { new NumericType { INTEGER } };
}

std::unique_ptr<BaseType> BaseType::newFloat() {
    return std::unique_ptr<BaseType> { new NumericType { FLOAT } };
}

std::unique_ptr<BaseType> BaseType::newVoid() {
    return std::unique_ptr<BaseType> { new Void { VOID } };
}

BaseType::BaseType(int sizeOrder) :
        sizeOrder { sizeOrder } {
}

BaseType::~BaseType() {
}

bool BaseType::isInteger() const noexcept {
    return *this == INTEGER;
}

bool BaseType::operator==(const BaseType& rhs) const noexcept {
    return sizeOrder == rhs.sizeOrder;
}

bool BaseType::operator!=(const BaseType& rhs) const noexcept {
    return !(*this == rhs);
}

} /* namespace ast */
