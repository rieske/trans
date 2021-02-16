#include "Register.h"

#include "Value.h"

namespace codegen {

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
    this->valueHeld->assignRegister(this);
}

void Register::free() {
    if (this->valueHeld) {
        this->valueHeld->removeRegister(this);
        this->valueHeld = nullptr;
    }
}

} // namespace codegen

