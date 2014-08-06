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

void Declaration::dereference(int dereferenceCount) {
    this->dereferenceCount += dereferenceCount;
    for (int i { 0 }; i < dereferenceCount; ++i) {
        type += "p";
    }
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
