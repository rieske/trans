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

void DirectDeclarator::setHolder(semantic_analyzer::ValueEntry holder) {
    this->holder = std::make_unique<semantic_analyzer::ValueEntry>(holder);
}

semantic_analyzer::ValueEntry* DirectDeclarator::getHolder() const {
    if (!holder) {
        throw std::runtime_error { "DirectDeclarator::getHolder() == nullptr" };
    }
    return holder.get();
}

} // namespace ast

