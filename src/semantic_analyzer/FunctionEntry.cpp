#include "FunctionEntry.h"

namespace semantic_analyzer {

FunctionEntry::FunctionEntry(std::string name, ast::FunctionType type, translation_unit::Context context) :
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

ast::FunctionType FunctionEntry::getType() const {
    return type;
}

std::size_t FunctionEntry::argumentCount() const {
    return arguments().size();
}

const std::vector<std::unique_ptr<ast::FundamentalType>>& FunctionEntry::arguments() const {
    return type.getArgumentTypes();
}

const ast::FundamentalType& FunctionEntry::returnType() const {
    return type.getReturnType();
}

} // namespace semantic_analyzer

