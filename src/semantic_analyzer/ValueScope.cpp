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

} // namespace

namespace semantic_analyzer {

int ValueScope::wordSlotsFor(const type::Type& type) {
    const int size = type.getSize();
    if (size <= 0) {
        return 1;
    }
    // Round up to 8-byte stack slots (MACHINE_WORD_SIZE).
    return (size + 7) / 8;
}

bool ValueScope::insertSymbol(std::string name, const type::Type& type, translation_unit::Context context, bool global) {
    if (localSymbols.find(name) != localSymbols.end()) {
        return false;
    }
    // Parameters live in `arguments` but share block scope with the function body (C).
    auto existingArgument = std::find_if(arguments.begin(), arguments.end(), EntryWithSameNameExists { name });
    if (existingArgument != arguments.end()) {
        return false;
    }
    const int index = nextLocalWordIndex;
    nextLocalWordIndex += wordSlotsFor(type);
    ValueEntry entry { name, type, false, context, index, global };
    localSymbols.insert(std::make_pair(name, entry));
    return true;
}

void ValueScope::insertFunctionArgument(std::string name, const type::Type& type, translation_unit::Context context) {
    auto existingArgument = std::find_if(arguments.begin(), arguments.end(), EntryWithSameNameExists { name });
    if (existingArgument == arguments.end()) {
        ValueEntry entry { name, type, false, context, static_cast<int>(arguments.size()) };
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

void ValueScope::setConstantInitializer(const std::string& name, long value) {
    localSymbols.at(name).setConstantInitializer(value);
}

ValueEntry ValueScope::createTemporarySymbol(type::Type type) {
    std::string tempName = generateTempName();
    const int index = nextLocalWordIndex;
    nextLocalWordIndex += wordSlotsFor(type);
    ValueEntry temp { tempName, type, true, translation_unit::Context { "", 0 }, index };
    localSymbols.insert(std::make_pair(tempName, temp));
    return temp;
}

std::map<std::string, ValueEntry> ValueScope::getSymbols() const {
    return localSymbols;
}

std::vector<ValueEntry> ValueScope::getArguments() const {
    return arguments;
}

} // namespace semantic_analyzer

