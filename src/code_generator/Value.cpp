#include "Value.h"

namespace codegen {

Value::Value(std::string name, int index, Type type, bool functionArgument) :
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

int Value::getIndex() const {
    return index;
}

Type Value::getType() const {
    return type;
}

} /* namespace code_generator */
