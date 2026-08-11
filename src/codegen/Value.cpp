#include "Value.h"

#include <cassert>

namespace codegen {

Value::Value(std::string name, int index, Type type, int sizeInBytes) :
        Value { std::move(name), index, type, sizeInBytes,
                type == Type::FLOATING ? type::sysv::sseScalar(sizeInBytes)
                : type == Type::COMPLEX ? type::sysv::complexClass(sizeInBytes)
                                       : type::sysv::integerScalar(sizeInBytes) }
{
}

Value::Value(std::string name, int index, Type type, int sizeInBytes,
        type::sysv::Classification classification) :
        name { std::move(name) },
        index { index },
        type { type },
        sizeInBytes { sizeInBytes },
        classification { classification }
{
}

std::string Value::getName() const {
    return name;
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

} // namespace codegen
