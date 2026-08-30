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

} // namespace

namespace semantic_analyzer {

int ValueScope::wordSlotsFor(const type::Type& type) {
    return type::object_abi::valueWords(type.getSize());
}

int ValueScope::allocateAutomatic(const type::Type& type) {
    return type::object_abi::takeAlignedWords(
            nextLocalWordIndex, type.getAlignment(), wordSlotsFor(type));
}

bool ValueScope::insertSymbol(SymbolKey key, const type::Type& type, translation_unit::Context context,
        symbols::Storage storage, std::string objectName, std::string sourceName) {
    if (localSymbols.find(key) != localSymbols.end()) {
        return false;
    }
    int index = 0;
    if (storage == symbols::Storage::Automatic) {
        index = allocateAutomatic(type);
    }
    symbols::ValueEntry entry {
            std::move(objectName), type, context, index, storage, std::move(sourceName) };
    localSymbols.insert(std::make_pair(std::move(key), entry));
    return true;
}

void ValueScope::insertFunctionArgument(std::string objectName, const type::Type& type,
        translation_unit::Context context, std::string sourceName) {
    auto existingArgument = std::find_if(arguments.begin(), arguments.end(),
            [&sourceName](const symbols::ValueEntry& entry) {
                return entry.sourceName() == sourceName;
            });
    if (existingArgument == arguments.end()) {
        symbols::ValueEntry entry {
                std::move(objectName), type, context, static_cast<int>(arguments.size()),
                symbols::Storage::Automatic, std::move(sourceName) };
        arguments.push_back(std::move(entry));
    }
}

symbols::ValueEntry ValueScope::lookup(const SymbolKey& key) const {
    return localSymbols.at(key);
}

bool ValueScope::contains(const SymbolKey& key) const {
    return localSymbols.find(key) != localSymbols.end();
}

const symbols::ValueEntry* ValueScope::findArgumentBySource(const std::string& source) const {
    auto it = std::find_if(arguments.begin(), arguments.end(),
            [&source](const symbols::ValueEntry& entry) {
                return entry.sourceName() == source;
            });
    if (it == arguments.end()) {
        return nullptr;
    }
    return &*it;
}

void ValueScope::setStaticInit(const SymbolKey& key, std::vector<symbols::StaticInitValue> words) {
    localSymbols.at(key).setStaticInit(std::move(words));
}

void ValueScope::promoteExternToDefinition(const SymbolKey& key) {
    localSymbols.at(key).promoteExternToDefinition();
}

void ValueScope::markDefiningInitializer(const SymbolKey& key) {
    localSymbols.at(key).markDefiningInitializer();
}

void ValueScope::refineType(const SymbolKey& key, const type::Type& type) {
    localSymbols.at(key).refineType(type);
}

symbols::ValueEntry ValueScope::createTemporarySymbol(type::Type type) {
    std::string tempName = generateTempName();
    const int index = allocateAutomatic(type);
    symbols::ValueEntry temp { tempName, type, translation_unit::Context { "", 0 }, index };
    temp.markExpressionTemp();
    localSymbols.insert(std::make_pair(SymbolKey { 0, tempName }, temp));
    return temp;
}

const std::map<SymbolKey, symbols::ValueEntry>& ValueScope::getSymbols() const {
    return localSymbols;
}

std::vector<symbols::ValueEntry> ValueScope::getArguments() const {
    return arguments;
}

} // namespace semantic_analyzer

