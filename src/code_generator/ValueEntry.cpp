#include "ValueEntry.h"

#include <iostream>
#include <sstream>

#include "ast/types/BaseType.h"

using ast::BaseType;

namespace code_generator {

ValueEntry::ValueEntry(std::string name, ast::Type type, bool tmp, unsigned l) :
        name { name },
        type { type },
        size { 4 },
        temp { tmp },
        param { false },
        line { l },
        offset { 0 },
        stored { true }
{
}

ValueEntry::~ValueEntry() {
}

ast::Type ValueEntry::getType() const {
    return type;
}

void ValueEntry::print() const {
    std::cout << "\t" << name << "\t" << (temp ? "temp" : "") << "\t" << offset << "\t" << getStorage() << "\t" << type.toString() << std::endl;
}

bool ValueEntry::isTemp() const {
    return temp;
}

unsigned ValueEntry::getLine() const {
    return line;
}

unsigned ValueEntry::getOffset() const {
    return offset;
}

void ValueEntry::setOffset(unsigned offset) {
    this->offset = offset;
}

std::string ValueEntry::getOffsetReg() const {
    if (param)
        return "ebp";
    else
        return "esp";
}

std::string ValueEntry::getStorage() const {
    std::ostringstream oss;
    if (param)
        oss << "[ebp";
    else
        oss << "[esp";
    if (offset)
        oss << " + " << offset;
    oss << "]";
    return oss.str();
}

void ValueEntry::setParam() {
    param = true;
}

void ValueEntry::update(std::string reg) {
    if ("" != reg) {
        value.push_back(reg);
        stored = false;
    } else {
        value.clear();
        stored = true;
    }
}

std::string ValueEntry::store() {
    std::string ret = "";
    if (!stored) {
        ret = "\tmov ";
        ret += getStorage();
        ret += ", ";
        ret += value.at(0);
    }
    return ret;
}

unsigned ValueEntry::getSize() const {
    return size;
}

bool ValueEntry::isStored() const {
    return stored;
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

std::string ValueEntry::getValue() const {
    if (value.size()) {
        return value.at(0);
    }
    return "";
}

} /* namespace code_generator */
