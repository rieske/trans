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

void Value::update(std::string reg) {
    if ("" != reg) {
        value.push_back(reg);
    } else {
        value.clear();
    }
}

bool Value::isStored() const {
    return value.empty();
}

void Value::removeReg(std::string reg) {
    std::vector<std::string> newVal;
    for (unsigned i = 0; i < value.size(); i++)
        if (value.at(i) != reg)
            newVal.push_back(value.at(i));
    value = newVal;
}

std::string Value::getValue() const {
    if (value.size()) {
        return value.front();
    }
    return "";
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
