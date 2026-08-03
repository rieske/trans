#include "DirectDeclarator.h"

#include "Expression.h"

namespace ast {

DirectDeclarator::DirectDeclarator(std::string name, const translation_unit::Context& context) :
        name { name },
        context { context }
{
}

std::string DirectDeclarator::getName() const {
    return name;
}

translation_unit::Context DirectDeclarator::getContext() const {
    return context;
}

void DirectDeclarator::foldArrayBoundSizeofs(const std::function<void(Expression*)>& foldSizeof) {
    (void)foldSizeof;
}

bool DirectDeclarator::hasArrayDeclarator() const {
    return false;
}

} // namespace ast
