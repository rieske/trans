#include "ValueScope.h"

#include <algorithm>
#include <stdexcept>

#include "translation_unit/Context.h"
#include "types/ObjectAbi.h"

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
    bool operator()(const symbols::ValueEntry& entry) {
        return entry.getName() == name;
    }

private:
    std::string name;
};

} // namespace

namespace semantic_analyzer {

bool ValueScope::insertSymbol(std::string name, const type::Type& type, translation_unit::Context context,
        symbols::Storage storage, std::string objectName) {
    auto existing = localSymbols.find(name);
    if (existing != localSymbols.end()) {
        return false;
    }
    // Parameters live in `arguments` but share block scope with the function body (C).
    auto existingArgument = std::find_if(arguments.begin(), arguments.end(), EntryWithSameNameExists { name });
    if (existingArgument != arguments.end()) {
        return false;
    }
    int index = 0;
    if (storage == symbols::Storage::Automatic) {
        index = nextLocalWordIndex;
        nextLocalWordIndex += type::object_abi::valueWords(type.getSize());
    }
    symbols::ValueEntry entry { std::move(objectName), type, context, index, storage };
    localSymbols.insert(std::make_pair(name, entry));
    return true;
}

void ValueScope::insertFunctionArgument(std::string name, const type::Type& type, translation_unit::Context context) {
    auto existingArgument = std::find_if(arguments.begin(), arguments.end(), EntryWithSameNameExists { name });
    if (existingArgument == arguments.end()) {
        symbols::ValueEntry entry { name, type, context, static_cast<int>(arguments.size()) };
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

symbols::ValueEntry ValueScope::lookup(std::string name) const {
    if (localSymbols.find(name) == localSymbols.end()) {
        auto existingArgument = std::find_if(arguments.begin(), arguments.end(), EntryWithSameNameExists { name });
        if (existingArgument == arguments.end()) {
            throw std::out_of_range("symbol not found: " + name);
        }
        return *existingArgument;
    }
    return localSymbols.at(name);
}

void ValueScope::setGlobalInitializer(const std::string& name, symbols::GlobalInitializer init) {
    localSymbols.at(name).setGlobalInitializer(std::move(init));
}

void ValueScope::setSymbolType(const std::string& name, const type::Type& type) {
    localSymbols.at(name).setType(type);
}

void ValueScope::promoteExternToDefinition(const std::string& name) {
    localSymbols.at(name).promoteExternToDefinition();
}

void ValueScope::markDefiningInitializer(const std::string& name) {
    localSymbols.at(name).markDefiningInitializer();
}

symbols::ValueEntry ValueScope::createTemporarySymbol(type::Type type) {
    std::string tempName = generateTempName();
    const int index = nextLocalWordIndex;
    nextLocalWordIndex += type::object_abi::valueWords(type.getSize());
    symbols::ValueEntry temp { tempName, type, translation_unit::Context { "", 0 }, index };
    temp.markExpressionTemp();
    localSymbols.insert(std::make_pair(tempName, temp));
    return temp;
}

std::map<std::string, symbols::ValueEntry> ValueScope::getSymbols() const {
    return localSymbols;
}

std::vector<symbols::ValueEntry> ValueScope::getArguments() const {
    return arguments;
}

} // namespace semantic_analyzer
