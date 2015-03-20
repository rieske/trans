#include "Register.h"

#include "Value.h"

namespace code_generator {

Register::Register(std::string name) :
        name { name }
{
}

std::string Register::getName() const {
    return name;
}

bool Register::containsUnstoredValue() const {
    return valueHeld && !valueHeld->isStored();
}

Value* Register::getValue() const {
    return valueHeld;
}

void Register::assign(Value* value) {
    this->valueHeld = value;
    this->valueHeld->assignRegister(name);
}

void Register::free() {
    this->valueHeld->removeReg(name);
    this->valueHeld = nullptr;
}

} /* namespace code_generator */
