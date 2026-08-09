#ifndef SYMBOLS_VALUEENTRY_H_
#define SYMBOLS_VALUEENTRY_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "translation_unit/Context.h"
#include "types/Type.h"

namespace symbols {

enum class Storage {
    Automatic,
    Global,
    Static,
    Extern
};

class ValueEntry {
public:
    ValueEntry(std::string name, const type::Type& type, translation_unit::Context context, int index,
            Storage storage = Storage::Automatic);

    std::string getName() const;
    bool isGlobal() const;
    bool isStatic() const;
    bool isExtern() const;
    type::Type getType() const;
    translation_unit::Context getContext() const;
    int getIndex() const;

    // Folded integer constant initializer for file-scope variables (unset means default 0).
    void setConstantInitializer(long value);
    std::optional<long> getConstantInitializer() const;

    // Multi-word .data operands for file-scope aggregates (NASM dq list).
    void setMultiWordInitializer(std::vector<std::string> words);
    const std::optional<std::vector<std::string>>& getMultiWordInitializer() const;

private:
    std::string name;
    type::Type type;
    translation_unit::Context context;
    int index;

    Storage storage;
    std::optional<long> constantInitializer;
    std::optional<std::vector<std::string>> multiWordInitializer;
};

} // namespace symbols

#endif // SYMBOLS_VALUEENTRY_H_
