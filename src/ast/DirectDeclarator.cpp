#include "DirectDeclarator.h"
#include "ArrayDeclarator.h"

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

bool DirectDeclarator::hasArrayDeclarator() const {
    bool found = false;
    const_cast<DirectDeclarator*>(this)->forEachArrayDeclarator(
            [&](ArrayDeclarator&) { found = true; });
    return found;
}

} // namespace ast

