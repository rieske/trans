#include "Declaration.h"

namespace semantic_analyzer {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(std::string name, std::string type) :
        name { name },
        type { type },
        lineNumber { lineNumber } {
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

size_t Declaration::getLineNumber() const {
    return lineNumber;
}

void Declaration::setHolder(SymbolEntry* holder) {
    this->holder = holder;
}

SymbolEntry* Declaration::getHolder() const {
    return holder;
}

}
