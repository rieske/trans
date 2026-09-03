#ifndef SYMBOLS_VALUEENTRY_H_
#define SYMBOLS_VALUEENTRY_H_

#include <string>
#include <vector>

#include "StaticInit.h"
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
            Storage storage = Storage::Automatic, std::string sourceName = {});

    const std::string& getName() const;
    const std::string& sourceName() const;
    bool isGlobal() const;
    bool isStatic() const;
    bool isExtern() const;
    // Only Extern -> Global (file-scope definition after a reference).
    void promoteExternToDefinition();
    // A file-scope declaration with an initializer, recorded at bind (not after lower).
    bool hasDefiningInitializer() const;
    void markDefiningInitializer();
    void markFunctionDefined();
    bool isFunctionDefined() const;
    type::Type getType() const;
    // File-scope redecl: replace with the C 6.2.7 composite type.
    void refineType(const type::Type& refined);
    void setContext(translation_unit::Context context);
    translation_unit::Context getContext() const;
    int getIndex() const;

    // Static-duration .data words. Empty means zero-fill.
    void setStaticInit(std::vector<StaticInitValue> words);
    const std::vector<StaticInitValue>& staticInit() const;

    bool isExpressionTemp() const { return expressionTemp_; }
    void markExpressionTemp() { expressionTemp_ = true; }

private:
    std::string name;
    std::string sourceName_;
    type::Type type;
    translation_unit::Context context;
    int index;

    Storage storage;
    bool definingInitializer { false };
    bool functionDefined_ { false };
    bool expressionTemp_ { false };
    std::vector<StaticInitValue> staticInitWords;
};

} // namespace symbols

#endif // SYMBOLS_VALUEENTRY_H_
