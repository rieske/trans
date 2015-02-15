#include "ValueScope.h"

#include <iostream>
#include <stdexcept>
#include <utility>

#include "../translation_unit/Context.h"

const std::string TEMP_PREFIX = "$t";

namespace {

unsigned nextTemp { 0 };

std::string generateTempName() {
    return TEMP_PREFIX + std::to_string(++nextTemp);
}

}

namespace semantic_analyzer {

bool ValueScope::insertSymbol(std::string name, const ast::FundamentalType& type, translation_unit::Context context) {
    if (localSymbols.find(name) != localSymbols.end()) {
        return false;
    }
    ValueEntry entry { name, type, false, context, localSymbols.size() };
    localSymbols.insert(std::make_pair(name, entry));
    return true;
}

void ValueScope::insertFunctionArgument(std::string name, const ast::FundamentalType& type, translation_unit::Context context) {
    try {
        arguments.at(name);
    } catch (std::out_of_range &ex) {
        ValueEntry entry { name, type, false, context, arguments.size() };
        entry.setParam();
        arguments.insert(std::make_pair(name, entry));
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
            throw std::out_of_range("symbol not found: " + name);
        }
        return arguments.at(name);
    }
    return localSymbols.at(name);
}

ValueEntry ValueScope::createTemporarySymbol(std::unique_ptr<ast::FundamentalType> type) {
    std::string tempName = generateTempName();
    // FIXME:
    ValueEntry temp { tempName, *type, true, translation_unit::Context { "", 0 }, localSymbols.size() };
    localSymbols.insert(std::make_pair(tempName, temp));
    return temp;
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

std::map<std::string, ValueEntry> ValueScope::getSymbols() const {
    return localSymbols;
}

std::map<std::string, ValueEntry> ValueScope::getArguments() const {
    return arguments;
}

} /* namespace semantic_analyzer */
