#include "DirectDeclarator.h"

#include <memory>

namespace ast {

const std::string DirectDeclarator::ID { "<direct_declarator>" };

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

} // namespace ast

