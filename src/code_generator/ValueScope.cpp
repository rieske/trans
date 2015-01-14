#include "ValueScope.h"

#include <iostream>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "../ast/types/Type.h"

const std::string TEMP_PREFIX = "_t";

namespace code_generator {

ValueScope::ValueScope(ValueScope* parentScope) :
        parentScope { parentScope }
{
}

ValueScope::~ValueScope() {
}

bool ValueScope::insertSymbol(std::string name, ast::Type type, translation_unit::Context context) {
    if (values.find(name) != values.end()) {
        return false;
    }
    ValueEntry entry { name, type, false, context, valueIndex };
    values.insert(std::make_pair(name, entry));
    ++valueIndex;
    return true;
}

void ValueScope::insertFunctionArgument(std::string name, ast::Type type, translation_unit::Context context) {
    try {
        values.at(name);
    } catch (std::out_of_range &ex) {
        ValueEntry entry { name, type, false, context, argumentIndex };
        entry.setParam();
        values.insert(std::make_pair(name, entry));
        ++argumentIndex;
    }
}

bool ValueScope::isSymbolDefined(std::string symbolName) const {
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
    // FIXME:
    ValueEntry temp { tempName, type, true, translation_unit::Context { "", 0 }, valueIndex };
    values.insert(std::make_pair(tempName, temp));
    ++valueIndex;
    return temp;
}

ValueScope* const ValueScope::getParentScope() const {
    return parentScope;
}

unsigned ValueScope::getTableSize() const {
    return values.size() - (argumentIndex - 2);
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
