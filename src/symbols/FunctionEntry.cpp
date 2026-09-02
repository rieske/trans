#include "FunctionEntry.h"

namespace symbols {

FunctionEntry::FunctionEntry(std::string name, type::Function type, translation_unit::Context context,
        bool internalLinkage) :
        name { std::move(name) },
        type { std::move(type) },
        context { std::move(context) },
        internalLinkage { internalLinkage }
{
}

translation_unit::Context FunctionEntry::getContext() const {
    return context;
}

bool FunctionEntry::hasInternalLinkage() const {
    return internalLinkage;
}

const std::string& FunctionEntry::getName() const {
    return name;
}

type::Function FunctionEntry::getType() const {
    return type;
}

const std::vector<type::Type>& FunctionEntry::arguments() const {
    return type.getArguments();
}

const type::Type& FunctionEntry::returnType() const {
    return type.getReturnType();
}

} // namespace symbols
