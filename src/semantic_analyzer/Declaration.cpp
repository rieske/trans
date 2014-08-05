#include "Declaration.h"

#include <vector>

namespace semantic_analyzer {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration() {
}

Declaration::Declaration(SymbolTable *st, unsigned ln) :
		NonterminalNode(st, ln) {
}

Declaration::~Declaration() {
}

void Declaration::dereference(int dereferenceCount) {
    this->dereferenceCount += dereferenceCount;
    for (int i {0}; i < dereferenceCount; ++i) {
        type += "p";
    }
}

string Declaration::getName() const {
	return name;
}

std::string Declaration::getType() const {
	return type;
}

}
