#ifndef VALUESCOPE_H_
#define VALUESCOPE_H_

#include <map>
#include <string>
#include <vector>

#include "symbols/ValueEntry.h"
#include "types/Type.h"

namespace semantic_analyzer {

// 0 is file scope; function scopes start at 1.
struct SymbolKey {
    unsigned scopeId { 0 };
    std::string source;

    bool operator<(const SymbolKey& other) const {
        return scopeId < other.scopeId
                || (scopeId == other.scopeId && source < other.source);
    }
};

class ValueScope {
public:
    bool insertSymbol(SymbolKey key, const type::Type& type, translation_unit::Context context,
            symbols::Storage storage, std::string objectName, std::string sourceName);
    void insertFunctionArgument(std::string objectName, const type::Type& type,
            translation_unit::Context context, std::string sourceName);
    symbols::ValueEntry createTemporarySymbol(type::Type type);
    symbols::ValueEntry lookup(const SymbolKey& key) const;
    bool contains(const SymbolKey& key) const;
    const symbols::ValueEntry* findArgumentBySource(const std::string& source) const;
    void setStaticInit(const SymbolKey& key, std::vector<symbols::StaticInitValue> words);
    void promoteExternToDefinition(const SymbolKey& key);
    void markDefiningInitializer(const SymbolKey& key);
    void refineType(const SymbolKey& key, const type::Type& type);

    const std::map<SymbolKey, symbols::ValueEntry>& getSymbols() const;
    std::vector<symbols::ValueEntry> getArguments() const;

private:
    // Next free stack-slot index in machine words
    // (type::object_abi::MACHINE_WORD_SIZE). Multi-word objects (arrays,
    // future aggregates) reserve consecutive slots.
    int nextLocalWordIndex { 0 };

    std::vector<symbols::ValueEntry> arguments;
    std::map<SymbolKey, symbols::ValueEntry> localSymbols;

    static int wordSlotsFor(const type::Type& type);
    int allocateAutomatic(const type::Type& type);
};

} // namespace semantic_analyzer

#endif // VALUESCOPE_H_
