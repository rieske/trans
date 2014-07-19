#include "Pointer.h"

#include <vector>

namespace semantic_analyzer {

const std::string Pointer::ID { "<ptr>" };

Pointer::Pointer(Pointer* pointer) :
		NonterminalNode(ID, { pointer }) {
	type = pointer->getType();
	type = "p" + type;
}

Pointer::Pointer() :
		NonterminalNode(ID, { }) {
	type = "p";
}

string Pointer::getType() const {
	return type;
}

}
