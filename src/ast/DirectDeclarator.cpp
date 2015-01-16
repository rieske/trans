#include "DirectDeclarator.h"

#include <memory>

namespace ast {

const std::string DirectDeclarator::ID { "<decl>" };

DirectDeclarator::DirectDeclarator(std::string name, const translation_unit::Context& context) :
        name { name },
        context { context }
{
}

DirectDeclarator::~DirectDeclarator() {
}

std::string DirectDeclarator::getName() const {
    return name;
}

int DirectDeclarator::getDereferenceCount() const {
    return 0;
}

translation_unit::Context DirectDeclarator::getContext() const {
    return context;
}

void DirectDeclarator::setHolder(code_generator::ValueEntry holder) {
    this->holder = std::make_unique<code_generator::ValueEntry>(holder);
}

code_generator::ValueEntry* DirectDeclarator::getHolder() const {
    return holder.get();
}

}
