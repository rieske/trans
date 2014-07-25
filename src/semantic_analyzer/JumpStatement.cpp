#include "JumpStatement.h"

#include <iostream>
#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string JumpStatement::ID { "<jmp_stmt>" };

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

void JumpStatement::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
