#ifndef SYMBOLS_VALUEENTRY_H_
#define SYMBOLS_VALUEENTRY_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "translation_unit/Context.h"
#include "types/Type.h"

namespace symbols {

class ValueEntry {
public:
    ValueEntry(std::string name, const type::Type& type, bool tmp, translation_unit::Context context, int index, bool global = false);

    std::string getName() const;
    bool isGlobal() const;
    type::Type getType() const;
    translation_unit::Context getContext() const;
    int getIndex() const;

    // Folded integer constant initializer for file-scope variables (unset means default 0).
    void setConstantInitializer(long value);
    std::optional<long> getConstantInitializer() const;

    // String literal initializer for file-scope char arrays (e.g. const char s[] = "hi").
    void setStringInitializer(std::string value);
    bool hasStringInitializer() const;
    const std::string& getStringInitializer() const;

    // Address constant initializer for file-scope pointers (e.g. int *p = &g).
    void setAddressInitializer(std::string symbolName);
    bool hasAddressInitializer() const;
    const std::string& getAddressInitializer() const;

    // Multi-word .data operands for brace-initialized globals (structs/arrays).
    // Each entry is a NASM dq operand (integer literal or symbol name).
    void setMultiWordInitializer(std::vector<std::string> words);
    bool hasMultiWordInitializer() const;
    const std::vector<std::string>& getMultiWordInitializer() const;

    // Replace type (file-scope incomplete array completion).
    void setType(const type::Type& newType);

    // File-scope linkage: extern has no storage in this TU; static is not exported.
    void setExternal(bool value);
    bool isExternal() const;
    void setStaticStorage(bool value);
    bool isStaticStorage() const;

    std::string to_string() const;

private:
    std::string name;
    type::Type type;
    translation_unit::Context context;
    int index;

    bool temp;
    bool global;
    bool external { false };
    bool staticStorage { false };
    std::optional<long> constantInitializer;
    std::optional<std::string> stringInitializer;
    std::optional<std::string> addressInitializer;
    std::optional<std::vector<std::string>> multiWordInitializer;
};

} // namespace symbols

#endif // SYMBOLS_VALUEENTRY_H_
