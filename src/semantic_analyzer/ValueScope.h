#ifndef VALUESCOPE_H_
#define VALUESCOPE_H_

#include <map>
#include <string>
#include <vector>

#include "symbols/ValueEntry.h"
#include "types/Type.h"

namespace semantic_analyzer {

class ValueScope {
public:
    bool insertSymbol(std::string name, const type::Type& type, translation_unit::Context context,
            symbols::Storage storage, std::string objectName);
    void insertFunctionArgument(std::string name, const type::Type& type, translation_unit::Context context);
    symbols::ValueEntry createTemporarySymbol(type::Type type);
    symbols::ValueEntry lookup(std::string name) const;
    bool contains(const std::string& name) const;
    void setStaticInit(const std::string& name, std::vector<symbols::StaticInitValue> words);
    void promoteExternToDefinition(const std::string& name);
    void markDefiningInitializer(const std::string& name);
    void refineType(const std::string& name, const type::Type& type);

    std::map<std::string, symbols::ValueEntry> getSymbols() const;
    std::vector<symbols::ValueEntry> getArguments() const;

private:
    // Next free stack-slot index in machine words
    // (type::object_abi::MACHINE_WORD_SIZE). Multi-word objects (arrays,
    // future aggregates) reserve consecutive slots.
    int nextLocalWordIndex { 0 };

    std::vector<symbols::ValueEntry> arguments;
    std::map<std::string, symbols::ValueEntry> localSymbols;

    static int wordSlotsFor(const type::Type& type);
};

} // namespace semantic_analyzer

#endif // VALUESCOPE_H_
