#include "Declaration.h"

#include <memory>

namespace ast {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(std::string name, const translation_unit::Context& context) :
        name { name },
        context { context }
{
}

Declaration::~Declaration() {
}

void Declaration::setDereferenceCount(int dereferenceCount) {
    this->dereferenceCount = dereferenceCount;
}

std::string Declaration::getName() const {
    return name;
}

int Declaration::getDereferenceCount() const {
    return dereferenceCount;
}

translation_unit::Context Declaration::getContext() const {
    return context;
}

void Declaration::setHolder(code_generator::ValueEntry holder) {
    this->holder = std::make_unique<code_generator::ValueEntry>(holder);
}

code_generator::ValueEntry* Declaration::getHolder() const {
    return holder.get();
}

}
