#include "JumpStatement.h"

#include <iostream>
#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

JumpStatement::JumpStatement(TerminalSymbol jumpKeyword) :
		jumpKeyword { jumpKeyword } {
	if (jumpKeyword.type == "continue") {
		std::cerr << jumpKeyword.type << " not implemented yet!\n";
	} else if (jumpKeyword.type == "break") {
		std::cerr << jumpKeyword.type << " not implemented yet!\n";
	} else {
		throw std::runtime_error { "bad loop jump keyword: " + jumpKeyword.type };
	}
}

void JumpStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} // namespace ast

