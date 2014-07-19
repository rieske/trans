#include "Declaration.h"

#include <vector>

#include "DirectDeclaration.h"
#include "Pointer.h"

namespace semantic_analyzer {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(Pointer* pointer, DirectDeclaration* directDeclaration) :
		NonterminalNode(ID, { pointer, directDeclaration }) {
	type = pointer->getType();
	name = directDeclaration->getName();
	type = type + directDeclaration->getType();
}

string Declaration::getName() const {
	return name;
}

string Declaration::getType() const {
	return type;
}

}
