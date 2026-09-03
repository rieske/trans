#ifndef SYMBOLS_FUNCTIONENTRY_H_
#define SYMBOLS_FUNCTIONENTRY_H_

#include <string>
#include <vector>

#include "translation_unit/Context.h"
#include "types/Function.h"
#include "types/Type.h"

namespace symbols {

class ValueEntry;

class FunctionEntry {
public:
    FunctionEntry(std::string name, type::Function type, translation_unit::Context context,
            bool internalLinkage = false);
    explicit FunctionEntry(const ValueEntry& value);

    const std::string& getName() const;
    type::Function getType() const;
    translation_unit::Context getContext() const;
    bool hasInternalLinkage() const;

    const std::vector<type::Type>& arguments() const;
    const type::Type& returnType() const;

private:
    std::string name;
    type::Function type;
    translation_unit::Context context;
    bool internalLinkage { false };
};

} // namespace symbols

#endif // SYMBOLS_FUNCTIONENTRY_H_
