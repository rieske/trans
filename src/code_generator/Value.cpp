#include "Value.h"

namespace code_generator {

Value::Value(std::string name, std::size_t index, Type type, bool functionArgument) :
        name { name },
        index { index },
        type { type },
        functionArgument { functionArgument }
{
}

std::string Value::getName() const {
    return name;
}

void Value::assignRegister(std::string reg) {
    assignedRegisterName = reg;
}

bool Value::isStored() const {
    return assignedRegisterName.empty();
}

void Value::removeReg(std::string reg) {
    if (reg == assignedRegisterName) {
        assignedRegisterName.clear();
    }
}

std::string Value::getAssignedRegisterName() const {
    return assignedRegisterName;
}

bool Value::isFunctionArgument() const {
    return functionArgument;
}

std::size_t Value::getIndex() const {
    return index;
}

Type Value::getType() const {
    return type;
}

} /* namespace code_generator */
