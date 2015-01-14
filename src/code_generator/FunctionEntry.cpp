#include "FunctionEntry.h"

namespace code_generator {

FunctionEntry::FunctionEntry(std::string name, ast::Function type, translation_unit::Context context) :
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

const std::string& FunctionEntry::getName() const {
    return name;
}

const ast::Function& FunctionEntry::getType() const {
    return type;
}

std::size_t FunctionEntry::argumentCount() const {
    return argumentTypes().size();
}

const std::vector<ast::Type>& FunctionEntry::argumentTypes() const {
    return type.getArgumentTypes();
}

const ast::Type& FunctionEntry::returnType() const {
    return type.getReturnType();
}

} /* namespace code_generator */
