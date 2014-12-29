#include "ValueScope.h"

#include <iostream>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "../ast/types/Type.h"

const std::string TEMP_PREFIX = "_t";
const unsigned VARIABLE_SIZE = 4;

namespace code_generator {

ValueScope::ValueScope(ValueScope* parentScope) :
        parentScope { parentScope }
{
}

ValueScope::~ValueScope() {
}

int ValueScope::insert(std::string name, ast::Type type, unsigned line) {
    try {
        return values.at(name).getLine();
    } catch (std::out_of_range &ex) {
        ValueEntry entry { name, type, false, line };
        entry.setOffset(offset);
        offset += VARIABLE_SIZE;
        values.insert(std::make_pair(name, entry));
    }
    return 0;
}

void ValueScope::insertFunctionArgument(std::string name, ast::Type type, unsigned line) {
    try {
        values.at(name);
    } catch (std::out_of_range &ex) {
        ValueEntry entry { name, type, false, line };
        entry.setOffset(paramOffset);
        paramOffset += VARIABLE_SIZE;
        entry.setParam();
        values.insert(std::make_pair(name, entry));
    }
}

bool ValueScope::hasSymbol(std::string symbolName) const {
    try {
        lookup(symbolName);
        return true;
    } catch (std::out_of_range &ex) {
        return false;
    }
}

ValueEntry ValueScope::lookup(std::string name) const {
    try {
        return values.at(name);
    } catch (std::out_of_range &ex) {
        if (parentScope) {
            return parentScope->lookup(name);
        }
        throw;
    }
}

ValueEntry ValueScope::newTemp(ast::Type type) {
    std::string tempName = generateTempName();
    ValueEntry temp { tempName, type, true, 0 };
    temp.setOffset(offset);
    offset += VARIABLE_SIZE;
    values.insert(std::make_pair(tempName, temp));
    return temp;
}

ValueScope* const ValueScope::getParentScope() const {
    return parentScope;
}

unsigned ValueScope::getTableSize() const {
    return values.size() * VARIABLE_SIZE - (paramOffset - 8);
}

void ValueScope::print() const {
    std::cout << "BEGIN SCOPE\t" << getTableSize() << std::endl;
    for (const auto& value : values) {
        value.second.print();
    }
    std::cout << "END SCOPE" << std::endl;
}

std::string ValueScope::generateTempName() {
    return TEMP_PREFIX + std::to_string(++nextTemp);
}

} /* namespace code_generator */
