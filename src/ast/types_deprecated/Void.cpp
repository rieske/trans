#include "Void.h"

#include <memory>

#include "NumericType.h"

namespace ast {

Void::Void() :
        BaseType { 0 } {
}

std::unique_ptr<BaseType> Void::clone() const {
    return std::unique_ptr<BaseType> { new Void { } };
}

Void::~Void() {
}

bool Void::canConvertTo(const BaseType& otherType) const noexcept {
    return otherType.canConvertTo(*this);
}

bool Void::canConvertTo(const NumericType&) const noexcept {
    return false;
}

bool Void::canConvertTo(const Void&) const noexcept {
    return true;
}

bool Void::canConvertTo(const Function&) const noexcept {
    return false;
}

std::string Void::toString() const {
    return "void";
}

bool Void::isEqual(const BaseType& otherType) const noexcept {
    return otherType.isEqual(*this);
}

} /* namespace ast */
