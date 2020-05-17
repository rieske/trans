#include "ValueScope.h"

#include <iostream>
#include <algorithm>

#include "translation_unit/Context.h"

const std::string TEMP_PREFIX = "$t";

namespace {

unsigned nextTemp { 0 };

std::string generateTempName() {
    return TEMP_PREFIX + std::to_string(++nextTemp);
}

class EntryWithSameNameExists {
public:
    EntryWithSameNameExists(std::string name) :
            name { name }
    {
    }
    bool operator()(const semantic_analyzer::ValueEntry& entry) {
        return entry.getName() == name;
    }

private:
    std::string name;
};

}

namespace semantic_analyzer {

bool ValueScope::insertSymbol(std::string name, const ast::FundamentalType& type, translation_unit::Context context) {
    if (localSymbols.find(name) != localSymbols.end()) {
        return false;
    }
    ValueEntry entry { name, type, false, context, static_cast<int>(localSymbols.size()) };
    localSymbols.insert(std::make_pair(name, entry));
    return true;
}

void ValueScope::insertFunctionArgument(std::string name, const ast::FundamentalType& type, translation_unit::Context context) {
    auto existingArgument = std::find_if(arguments.begin(), arguments.end(), EntryWithSameNameExists { name });
    if (existingArgument == arguments.end()) {
        ValueEntry entry { name, type, false, context, static_cast<int>(arguments.size()) };
        entry.setParam();
        arguments.push_back(entry);
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
        auto existingArgument = std::find_if(arguments.begin(), arguments.end(), EntryWithSameNameExists { name });
        if (existingArgument == arguments.end()) {
            throw std::out_of_range("symbol not found: " + name);
        }
        return *existingArgument;
    }
    return localSymbols.at(name);
}

ValueEntry ValueScope::createTemporarySymbol(std::unique_ptr<ast::FundamentalType> type) {
    std::string tempName = generateTempName();
// FIXME:
    ValueEntry temp { tempName, *type, true, translation_unit::Context { "", 0 }, static_cast<int>(localSymbols.size()) };
    localSymbols.insert(std::make_pair(tempName, temp));
    return temp;
}

void ValueScope::print() const {
    std::cout << "BEGIN SCOPE\n"
            << "--arguments--\n";
    for (const auto& value : arguments) {
        value.print();
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

std::vector<ValueEntry> ValueScope::getArguments() const {
    return arguments;
}

} /* namespace semantic_analyzer */
