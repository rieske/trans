#include "Value.h"

#include <cassert>
#include <stdexcept>

namespace codegen {

Value::Value(int id, int index, Type type, int sizeInBytes) :
        Value { id, index, type, sizeInBytes,
                type == Type::FLOATING ? type::sysv::sseScalar(sizeInBytes)
                : type == Type::COMPLEX ? type::sysv::complexClass(sizeInBytes)
                                       : type::sysv::integerScalar(sizeInBytes) }
{
}

Value::Value(int id, int index, Type type, int sizeInBytes,
        type::sysv::Classification classification) :
        id_ { id },
        index { index },
        type { type },
        sizeInBytes { sizeInBytes },
        classification { classification }
{
    if (id_ < 0) {
        throw std::logic_error { "Value requires a valid intern id" };
    }
}

Value Value::withIndex(int newIndex) const {
    Value copy = *this;
    copy.index = newIndex;
    copy.assignedRegister = nullptr;
    return copy;
}

int Value::id() const {
    return id_;
}

void Value::assignRegister(Register* reg) {
    assignedRegister = reg;
}

bool Value::isStored() const {
    return !assignedRegister;
}

void Value::removeRegister(Register* reg) {
    if (reg == assignedRegister) {
        assignedRegister = nullptr;
    }
}

Register& Value::getAssignedRegister() const {
    assert(assignedRegister != nullptr);
    return *assignedRegister;
}

int Value::getIndex() const {
    return index;
}

Type Value::getType() const {
    return type;
}

int Value::getSizeInBytes() const {
    return sizeInBytes;
}

type::sysv::Classification Value::getClassification() const {
    return classification;
}

void Value::markExpressionTemp() {
    expressionTemp_ = true;
}

bool Value::isExpressionTemp() const {
    return expressionTemp_;
}

void Value::setLastUseOrdinal(int ordinal) {
    lastUseOrdinal_ = ordinal;
}

int Value::getLastUseOrdinal() const {
    return lastUseOrdinal_;
}

} // namespace codegen
