#ifndef SYMBOLS_VALUEENTRY_H_
#define SYMBOLS_VALUEENTRY_H_

#include <optional>
#include <string>

#include "GlobalInitializer.h"
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
    // Only Extern -> Global (file-scope definition after a reference).
    void promoteExternToDefinition();
    // A file-scope declaration with an initializer, recorded at bind (not after lower).
    bool hasDefiningInitializer() const;
    void markDefiningInitializer();
    type::Type getType() const;
    translation_unit::Context getContext() const;
    int getIndex() const;

    // Sole write path for static-storage initializers (file-scope and function-scope static).
    void setGlobalInitializer(GlobalInitializer init);
    const GlobalInitializer* globalInitializer() const { return initializer_ ? &*initializer_ : nullptr; }

    void setType(const type::Type& newType);

private:
    std::string name;
    type::Type type;
    translation_unit::Context context;
    int index;

    Storage storage;
    bool definingInitializer { false };
    std::optional<GlobalInitializer> initializer_;
};

} // namespace symbols

#endif // SYMBOLS_VALUEENTRY_H_
