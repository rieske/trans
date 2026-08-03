#ifndef SYMBOLS_VALUEENTRY_H_
#define SYMBOLS_VALUEENTRY_H_

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "translation_unit/Context.h"
#include "types/Type.h"

namespace symbols {

// Closed file-scope initializer payload (not a bag of independent optionals).
struct ConstantInit {
    long value { 0 };
};
struct StringInit {
    std::string value;
};
struct AddressInit {
    std::string symbolName;
};
struct MultiWordInit {
    std::vector<std::string> words; // NASM dq operands
};

using GlobalInitializer = std::variant<ConstantInit, StringInit, AddressInit, MultiWordInit>;

class ValueEntry {
public:
    ValueEntry(std::string name, const type::Type& type, bool tmp, translation_unit::Context context, int index, bool global = false);

    std::string getName() const;
    bool isGlobal() const;
    type::Type getType() const;
    translation_unit::Context getContext() const;
    int getIndex() const;

    // Sole write path for file-scope initializers.
    void setGlobalInitializer(GlobalInitializer init);
    const GlobalInitializer* globalInitializer() const { return initializer_ ? &*initializer_ : nullptr; }

    // Convenience writers used by SymbolTable (forward to setGlobalInitializer).
    void setConstantInitializer(long value);
    void setStringInitializer(std::string value);
    void setAddressInitializer(std::string symbolName);
    void setMultiWordInitializer(std::vector<std::string> words);

    void setType(const type::Type& newType);

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
    std::optional<GlobalInitializer> initializer_;
};

} // namespace symbols

#endif // SYMBOLS_VALUEENTRY_H_
