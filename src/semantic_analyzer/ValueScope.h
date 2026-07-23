#ifndef VALUESCOPE_H_
#define VALUESCOPE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ValueEntry.h"
#include "types/Type.h"

namespace semantic_analyzer {

class ValueScope {
public:
    bool insertSymbol(std::string name, const type::Type& type, translation_unit::Context context, bool global = false);
    void insertFunctionArgument(std::string name, const type::Type& type, translation_unit::Context context);
    ValueEntry createTemporarySymbol(type::Type type);
    bool isSymbolDefined(std::string symbolName) const;
    ValueEntry lookup(std::string name) const;
    void setConstantInitializer(const std::string& name, long value);
    void setStringInitializer(const std::string& name, std::string value);
    void setAddressInitializer(const std::string& name, std::string symbolName);
    void setMultiWordInitializer(const std::string& name, std::vector<std::string> words);
    void setSymbolType(const std::string& name, const type::Type& type);
    // File-scope linkage flags (no-op if name is not in this scope).
    void setExternal(const std::string& name, bool value);
    void setStaticStorage(const std::string& name, bool value);

    std::map<std::string, ValueEntry> getSymbols() const;
    std::vector<ValueEntry> getArguments() const;

private:
    std::vector<ValueEntry> arguments;
    std::map<std::string, ValueEntry> localSymbols;
};

} // namespace semantic_analyzer

#endif // VALUESCOPE_H_
