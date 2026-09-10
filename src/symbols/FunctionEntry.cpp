#include "FunctionEntry.h"

#include <stdexcept>

#include "ValueEntry.h"

namespace symbols {

FunctionEntry::FunctionEntry(std::string name, type::Type type, translation_unit::Context context,
        bool internalLinkage) :
        name { std::move(name) },
        type { std::move(type) },
        context { std::move(context) },
        internalLinkage { internalLinkage },
        providesExternalDefinition_ { !internalLinkage }
{
    if (!this->type.isFunction()) {
        throw std::logic_error { "FunctionEntry requires a function type" };
    }
}

FunctionEntry::FunctionEntry(const ValueEntry& value) :
        FunctionEntry { value.getName(), value.getType(), value.getContext(),
                value.isStatic() }
{
    noreturn_ = value.isNoreturn();
    providesExternalDefinition_ = value.providesExternalDefinition();
}

translation_unit::Context FunctionEntry::getContext() const {
    return context;
}

bool FunctionEntry::hasInternalLinkage() const {
    return internalLinkage;
}

bool FunctionEntry::isNoreturn() const {
    return noreturn_;
}

bool FunctionEntry::providesExternalDefinition() const {
    return providesExternalDefinition_;
}

const std::string& FunctionEntry::getName() const {
    return name;
}

const type::Type& FunctionEntry::getType() const {
    return type;
}

const type::Function& FunctionEntry::function() const {
    return type.getFunction();
}

const std::vector<type::Type>& FunctionEntry::arguments() const {
    return function().getArguments();
}

const type::Type& FunctionEntry::returnType() const {
    return function().getReturnType();
}

bool FunctionEntry::isVariadic() const {
    return function().isVariadic();
}

} // namespace symbols
