#ifndef SYMBOLS_FUNCTIONENTRY_H_
#define SYMBOLS_FUNCTIONENTRY_H_

#include <string>
#include <vector>

#include "types/Function.h"
#include "translation_unit/Context.h"

namespace symbols {

class FunctionEntry {
public:
    FunctionEntry(std::string name, type::Function type, translation_unit::Context context,
            bool internalLinkage = false);

    std::string getName() const;
    type::Function getType() const;
    translation_unit::Context getContext() const;
    bool hasInternalLinkage() const;
    bool isDefined() const { return defined; }
    void markDefined() { defined = true; }

    std::size_t argumentCount() const;
    std::vector<type::Type> arguments() const;
    type::Type returnType() const;

private:
    std::string name;
    type::Function type;
    translation_unit::Context context;
    bool internalLinkage { false };
    bool defined { false };
};

} // namespace symbols

#endif // SYMBOLS_FUNCTIONENTRY_H_
