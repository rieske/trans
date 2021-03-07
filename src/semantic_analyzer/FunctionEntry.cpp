#include "FunctionEntry.h"
#include "types/Type.h"

namespace semantic_analyzer {

FunctionEntry::FunctionEntry(std::string name, type::Function type, translation_unit::Context context) :
        name { name },
        type { type },
        context { context }
{
}

FunctionEntry::~FunctionEntry() {
}

translation_unit::Context FunctionEntry::getContext() const {
    return context;
}

std::string FunctionEntry::getName() const {
    return name;
}

type::Function FunctionEntry::getType() const {
    return type;
}

std::size_t FunctionEntry::argumentCount() const {
    return arguments().size();
}

std::vector<type::Type> FunctionEntry::arguments() const {
    return type.getArguments();
}

type::Type FunctionEntry::returnType() const {
    return type.getReturnType();
}

} // namespace semantic_analyzer

