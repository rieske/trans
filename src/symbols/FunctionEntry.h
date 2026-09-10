#ifndef SYMBOLS_FUNCTIONENTRY_H_
#define SYMBOLS_FUNCTIONENTRY_H_

#include <string>
#include <vector>

#include "translation_unit/Context.h"
#include "types/Type.h"

namespace symbols {

class ValueEntry;

class FunctionEntry {
public:
    FunctionEntry(std::string name, type::Type type, translation_unit::Context context,
            bool internalLinkage = false);
    explicit FunctionEntry(const ValueEntry& value);

    const std::string& getName() const;
    const type::Type& getType() const;
    const type::Function& function() const;
    translation_unit::Context getContext() const;
    bool hasInternalLinkage() const;
    bool isNoreturn() const;
    bool providesExternalDefinition() const;

    const std::vector<type::Type>& arguments() const;
    const type::Type& returnType() const;
    bool isVariadic() const;

private:
    std::string name;
    type::Type type;
    translation_unit::Context context;
    bool internalLinkage { false };
    bool noreturn_ { false };
    bool providesExternalDefinition_ { true };
};

} // namespace symbols

#endif // SYMBOLS_FUNCTIONENTRY_H_
