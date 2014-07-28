#include "Pointer.h"

#include <vector>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string Pointer::ID { "<ptr>" };

Pointer::Pointer() :
		NonterminalNode(ID, { }) {
	type = "p";
}

void Pointer::dereference() {
	type = "p" + type;
}

std::string Pointer::getType() const {
	return type;
}

void Pointer::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
