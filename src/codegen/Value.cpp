#include "Value.h"

#include <cassert>

namespace codegen {

Value::Value(std::string name, int index, Type type, int sizeInBytes, bool isSigned) :
        name { name },
        index { index },
        type { type },
        sizeInBytes { sizeInBytes },
        signedIntegral { isSigned }
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

bool Value::isSigned() const {
    return signedIntegral;
}

void Value::setLastUseOrdinal(int ordinal) {
    lastUseOrdinal = ordinal;
}

int Value::getLastUseOrdinal() const {
    return lastUseOrdinal;
}

} // namespace codegen
