#include "Declaration.h"

namespace ast {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(std::string name, std::string type, const TranslationUnitContext& context) :
        name { name },
        type { type },
        context { context } {
}

Declaration::~Declaration() {
}

void Declaration::dereference(std::string pointerType) {
    type += pointerType;
}

std::string Declaration::getName() const {
    return name;
}

std::string Declaration::getType() const {
    return type;
}

const TranslationUnitContext& Declaration::getContext() const {
    return context;
}

void Declaration::setHolder(SymbolEntry* holder) {
    this->holder = holder;
}

SymbolEntry* Declaration::getHolder() const {
    return holder;
}

}
