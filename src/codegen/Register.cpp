#include "Register.h"

#include "Value.h"

#include <utility>

namespace codegen {

Register::Register(std::string name) :
        name { std::move(name) }
{
}

const std::string& Register::getName() const {
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

