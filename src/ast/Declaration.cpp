#include "Declaration.h"

namespace ast {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(std::string name, const TranslationUnitContext& context) :
        name { name },
        context { context } {
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

const TranslationUnitContext& Declaration::getContext() const {
    return context;
}

void Declaration::setHolder(code_generator::ValueEntry* holder) {
    this->holder = holder;
}

code_generator::ValueEntry* Declaration::getHolder() const {
    return holder;
}

}
