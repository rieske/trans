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

bool ValueScope::insertSymbol(std::string name, const type::Type& type, translation_unit::Context context, bool global) {
    auto existing = localSymbols.find(name);
    if (existing != localSymbols.end()) {
        if (!global) {
            return false;
        }
        // File-scope: C allows multiple compatible declarations of the same object.
        const type::Type& existingType = existing->second.getType();
        if (existingType.equivalentTo(type)) {
            return true;
        }
        // Compatible array redeclarations (git refs: extern T a[N]; T a[] = {...}).
        if (existingType.isArray() && type.isArray()
                && existingType.getElementType().equivalentTo(type.getElementType())) {
            if (existingType.getArraySize() == 0 && type.getArraySize() > 0) {
                existing->second.setType(type);
                return true;
            }
            if (type.getArraySize() == 0) {
                // Definition uses T name[] (size from initializer); keep prior size if any.
                return true;
            }
            if (existingType.getArraySize() == type.getArraySize()) {
                return true;
            }
        }
        return false;
    }
    // Parameters live in `arguments` but share block scope with the function body (C).
    auto existingArgument = std::find_if(arguments.begin(), arguments.end(), EntryWithSameNameExists { name });
    if (existingArgument != arguments.end()) {
        return false;
    }
    ValueEntry entry { name, type, false, context, static_cast<int>(localSymbols.size()), global };
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
    if (localSymbols.find(symbolName) != localSymbols.end()) {
        return true;
    }
    return std::find_if(arguments.begin(), arguments.end(), EntryWithSameNameExists { symbolName })
            != arguments.end();
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

void ValueScope::setStringInitializer(const std::string& name, std::string value) {
    localSymbols.at(name).setStringInitializer(std::move(value));
}

void ValueScope::setAddressInitializer(const std::string& name, std::string symbolName) {
    localSymbols.at(name).setAddressInitializer(std::move(symbolName));
}

void ValueScope::setMultiWordInitializer(const std::string& name, std::vector<std::string> words) {
    localSymbols.at(name).setMultiWordInitializer(std::move(words));
}

void ValueScope::setSymbolType(const std::string& name, const type::Type& type) {
    localSymbols.at(name).setType(type);
}

void ValueScope::setExternal(const std::string& name, bool value) {
    auto it = localSymbols.find(name);
    if (it != localSymbols.end()) {
        it->second.setExternal(value);
    }
}

void ValueScope::setStaticStorage(const std::string& name, bool value) {
    auto it = localSymbols.find(name);
    if (it != localSymbols.end()) {
        it->second.setStaticStorage(value);
    }
}

ValueEntry ValueScope::createTemporarySymbol(type::Type type) {
    std::string tempName = generateTempName();
    ValueEntry temp { tempName, type, true, translation_unit::Context { "", 0 }, static_cast<int>(localSymbols.size()) };
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

