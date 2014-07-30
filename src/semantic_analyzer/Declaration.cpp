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

void Declaration::dereference(std::string pointerType) {
	type = pointerType + type;
}

string Declaration::getName() const {
	return name;
}

string Declaration::getType() const {
	return type;
}

}
