#include "DirectDeclaration.h"

#include <memory>

namespace ast {

const std::string DirectDeclaration::ID { "<decl>" };

DirectDeclaration::DirectDeclaration(std::string name, const translation_unit::Context& context) :
        name { name },
        context { context }
{
}

DirectDeclaration::~DirectDeclaration() {
}

std::string DirectDeclaration::getName() const {
    return name;
}

int DirectDeclaration::getDereferenceCount() const {
    return 0;
}

translation_unit::Context DirectDeclaration::getContext() const {
    return context;
}

void DirectDeclaration::setHolder(code_generator::ValueEntry holder) {
    this->holder = std::make_unique<code_generator::ValueEntry>(holder);
}

code_generator::ValueEntry* DirectDeclaration::getHolder() const {
    return holder.get();
}

}
