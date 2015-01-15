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
    if (localSymbols.find(name) != localSymbols.end()) {
        return false;
    }
    ValueEntry entry { name, type, false, context, valueIndex };
    localSymbols.insert(std::make_pair(name, entry));
    ++valueIndex;
    return true;
}

void ValueScope::insertFunctionArgument(std::string name, ast::Type type, translation_unit::Context context) {
    try {
        arguments.at(name);
    } catch (std::out_of_range &ex) {
        ValueEntry entry { name, type, false, context, argumentIndex };
        entry.setParam();
        arguments.insert(std::make_pair(name, entry));
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
    if (localSymbols.find(name) == localSymbols.end()) {
        if (arguments.find(name) == arguments.end()) {
            if (parentScope) {
                return parentScope->lookup(name);
            }
            throw std::out_of_range("symbol not found");
        }
        return arguments.at(name);
    }
    return localSymbols.at(name);
}

ValueEntry ValueScope::newTemp(ast::Type type) {
    std::string tempName = generateTempName();
    // FIXME:
    ValueEntry temp { tempName, type, true, translation_unit::Context { "", 0 }, valueIndex };
    localSymbols.insert(std::make_pair(tempName, temp));
    ++valueIndex;
    return temp;
}

ValueScope* const ValueScope::getParentScope() const {
    return parentScope;
}

void ValueScope::print() const {
    std::cout << "BEGIN SCOPE\n"
            << "--arguments--\n";
    for (const auto& value : arguments) {
        value.second.print();
    }
    std::cout << "--locals--\n";
    for (const auto& value : localSymbols) {
        value.second.print();
    }
    std::cout << "END SCOPE" << std::endl;
}

std::map<std::string, ValueEntry> ValueScope::getSymbols() const{
    return localSymbols;
}

std::map<std::string, ValueEntry> ValueScope::getArguments() const{
    return arguments;
}

std::string ValueScope::generateTempName() {
    return TEMP_PREFIX + std::to_string(++nextTemp);
}

} /* namespace code_generator */
