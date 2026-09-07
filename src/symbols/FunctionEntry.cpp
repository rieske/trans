#include "FunctionEntry.h"

#include <stdexcept>

#include "ValueEntry.h"

namespace symbols {

FunctionEntry::FunctionEntry(std::string name, type::Type type, translation_unit::Context context,
        bool internalLinkage) :
        name { std::move(name) },
        type { std::move(type) },
        context { std::move(context) },
        internalLinkage { internalLinkage }
{
    if (!this->type.isFunction()) {
        throw std::logic_error { "FunctionEntry requires a function type" };
    }
}

FunctionEntry::FunctionEntry(const ValueEntry& value) :
        FunctionEntry { value.getName(), value.getType(), value.getContext(),
                value.isStatic() }
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

type::Type FunctionEntry::getType() const {
    return type;
}

const std::vector<type::Type>& FunctionEntry::arguments() const {
    return type.getFunction().getArguments();
}

const type::Type& FunctionEntry::returnType() const {
    return type.getFunction().getReturnType();
}

bool FunctionEntry::isVariadic() const {
    return type.getFunction().isVariadic();
}

} // namespace symbols
