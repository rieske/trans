#include "DirectDeclarator.h"

#include <utility>

namespace ast {

DirectDeclarator::DirectDeclarator(std::string name, const translation_unit::Context& context) :
        name { std::move(name) },
        context { context }
{
}

const std::string& DirectDeclarator::getName() const {
    return name;
}

translation_unit::Context DirectDeclarator::getContext() const {
    return context;
}

} // namespace ast

