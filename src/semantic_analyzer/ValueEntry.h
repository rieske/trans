#ifndef VALUEENTRY_H_
#define VALUEENTRY_H_

#include <memory>
#include <optional>
#include <string>

#include "translation_unit/Context.h"
#include "types/Type.h"

namespace semantic_analyzer {

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

    std::string to_string() const;

private:
    std::string name;
    type::Type type;
    translation_unit::Context context;
    int index;

    bool temp;
    bool global;
    std::optional<long> constantInitializer;
};

} // namespace semantic_analyzer

#endif // VALUEENTRY_H_
