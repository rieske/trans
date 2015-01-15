#include "ValueEntry.h"

#include <iostream>
#include <sstream>

#include "ast/types/BaseType.h"

using ast::BaseType;

namespace code_generator {

ValueEntry::ValueEntry(std::string name, ast::Type type, bool tmp, translation_unit::Context context, int index) :
        name { name },
        type { type },
        context { context },
        index { index },
        temp { tmp },
        param { false }
{
}

ValueEntry::~ValueEntry() {
}

ast::Type ValueEntry::getType() const {
    return type;
}

void ValueEntry::print() const {
    std::cout << "\t" << name << "\t" << (temp ? "temp" : "") << "\t" << index << "\t" << type.toString() << std::endl;
}

translation_unit::Context ValueEntry::getContext() const {
    return context;
}

int ValueEntry::getIndex() const {
    return index;
}

void ValueEntry::setParam() {
    param = true;
}

void ValueEntry::update(std::string reg) {
    if ("" != reg) {
        value.push_back(reg);
    } else {
        value.clear();
    }
}

bool ValueEntry::isStored() const {
    return value.empty();
}

void ValueEntry::removeReg(std::string reg) {
    std::vector<std::string> newVal;
    for (unsigned i = 0; i < value.size(); i++)
        if (value.at(i) != reg)
            newVal.push_back(value.at(i));
    value = newVal;
}

std::string ValueEntry::getName() const {
    return name;
}

bool ValueEntry::isFunctionArgument() const {
    return param;
}

std::string ValueEntry::getValue() const {
    if (value.size()) {
        return value.at(0);
    }
    return "";
}

} /* namespace code_generator */
