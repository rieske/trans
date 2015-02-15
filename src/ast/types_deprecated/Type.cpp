#include "Type.h"

#include <algorithm>

#include "NumericType.h"
#include "Void.h"

namespace ast {

Type::Type(std::unique_ptr<BaseType> baseType, int dereferenceCount) :
        baseType { std::move(baseType) },
        dereferenceCount { dereferenceCount } {
    if (dereferenceCount < 0) {
        throw std::invalid_argument { "dereference count can not be negative" };
    }
}

Type::Type(const Type& copyFrom) {
    this->dereferenceCount = copyFrom.dereferenceCount;
    this->baseType = copyFrom.baseType->clone();
}

Type::~Type() {
}

Type& Type::operator=(const Type& assignFrom) {
    if (&assignFrom != this) {
        this->dereferenceCount = assignFrom.dereferenceCount;
        this->baseType = assignFrom.baseType->clone();
    }
    return *this;
}

bool Type::operator==(const Type& rhs) const noexcept {
    return *this->baseType == *rhs.baseType && this->dereferenceCount == rhs.dereferenceCount;
}

bool Type::operator!=(const Type& rhs) const noexcept {
    return !(*this == rhs);
}

bool Type::isPointer() const noexcept {
    return dereferenceCount;
}

bool Type::isPlainVoid() const noexcept {
    return !isPointer() && *baseType == BaseType::VOID;
}

bool Type::isPlainInteger() const noexcept {
    return !isPointer() && *baseType == BaseType::INTEGER;
}

bool Type::canConvertTo(const Type& type) const noexcept {
    return this->dereferenceCount == type.dereferenceCount && this->baseType->canConvertTo(*type.baseType);
}

Type Type::getAddressType() const {
    return {baseType->clone(), dereferenceCount+1};
}

Type Type::getTypePointedTo() const {
    return {baseType->clone(), dereferenceCount-1};
}

std::string Type::toString() const {
    std::string pointerString;
    for (int i = 0; i < dereferenceCount; ++i) {
        pointerString += "*";
    }
    return baseType->toString() + pointerString;
}

}
