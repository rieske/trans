#include "Value.h"

#include <cassert>

namespace codegen {

Value::Value(std::string name, int index, Type type, int sizeInBytes, bool functionArgument, bool global) :
        name { name },
        index { index },
        type { type },
        sizeInBytes { sizeInBytes },
        functionArgument { functionArgument },
        global { global }
{
}

std::string Value::getName() const {
    return name;
}

void Value::assignRegister(Register* reg) {
    assignedRegister = reg;
}

bool Value::isStored() const {
    // A global's home is [rel name]; it is never register-resident, so it is always "stored".
    return global || !assignedRegister;
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

bool Value::isFunctionArgument() const {
    return functionArgument;
}

bool Value::isGlobal() const {
    return global;
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

} // namespace codegen

