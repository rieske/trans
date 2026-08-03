#ifndef VALUESCOPE_H_
#define VALUESCOPE_H_

#include <map>
#include <string>
#include <vector>

#include "symbols/GlobalInitializer.h"
#include "symbols/ValueEntry.h"
#include "types/Type.h"

namespace semantic_analyzer {

class ValueScope {
public:
    bool insertSymbol(std::string name, const type::Type& type, translation_unit::Context context,
            symbols::Storage storage, std::string objectName);
    void insertFunctionArgument(std::string name, const type::Type& type, translation_unit::Context context);
    symbols::ValueEntry createTemporarySymbol(type::Type type);
    bool isSymbolDefined(std::string name) const;
    symbols::ValueEntry lookup(std::string name) const;
    void setGlobalInitializer(const std::string& name, symbols::GlobalInitializer init);
    void setSymbolType(const std::string& name, const type::Type& type);
    void promoteExternToDefinition(const std::string& name);
    void markDefiningInitializer(const std::string& name);

    std::map<std::string, symbols::ValueEntry> getSymbols() const;
    std::vector<symbols::ValueEntry> getArguments() const;

private:
    // Next free stack-slot index in machine words (8-byte units). Multi-word
    // objects (arrays, aggregates) reserve consecutive slots.
    int nextLocalWordIndex { 0 };

    std::vector<symbols::ValueEntry> arguments;
    std::map<std::string, symbols::ValueEntry> localSymbols;
};


} // namespace semantic_analyzer

#endif // VALUESCOPE_H_
