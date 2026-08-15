#ifndef SYMBOLS_FUNCTIONENTRY_H_
#define SYMBOLS_FUNCTIONENTRY_H_

#include <string>
#include <vector>

#include "translation_unit/Context.h"
#include "types/Function.h"
#include "types/Type.h"

namespace symbols {

class FunctionEntry {
public:
    FunctionEntry(std::string name, type::Function type, translation_unit::Context context,
            bool internalLinkage = false);

    const std::string& getName() const;
    type::Function getType() const;
    translation_unit::Context getContext() const;
    bool hasInternalLinkage() const;

    std::size_t argumentCount() const;
    std::vector<type::Type> arguments() const;
    type::Type returnType() const;

private:
    std::string name;
    type::Function type;
    translation_unit::Context context;
    bool internalLinkage { false };
};

} // namespace symbols

#endif // SYMBOLS_FUNCTIONENTRY_H_
