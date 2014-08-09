#include "Declaration.h"

#include <vector>

namespace semantic_analyzer {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(std::string name, std::string type, size_t lineNumber, SymbolTable* st) :
        NonterminalNode(st, lineNumber),
        name { name },
        type { type },
        lineNumber { lineNumber } {
}

Declaration::~Declaration() {
}

void Declaration::dereference(std::string pointerType) {
    type += pointerType;
}

string Declaration::getName() const {
    return name;
}

std::string Declaration::getType() const {
    return type;
}

size_t Declaration::getLineNumber() const {
    return lineNumber;
}

}
