#include "JumpStatement.h"

#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

JumpStatement::JumpStatement(TerminalSymbol jumpKeyword) :
		jumpKeyword { jumpKeyword } {
	if (jumpKeyword.type != "continue" && jumpKeyword.type != "break") {
		throw std::runtime_error { "bad loop jump keyword: " + jumpKeyword.type };
	}
}

void JumpStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

void JumpStatement::setJumpTo(semantic_analyzer::LabelEntry label) {
	jumpTo = std::make_unique<semantic_analyzer::LabelEntry>(label);
}

semantic_analyzer::LabelEntry* JumpStatement::getJumpTo() const {
	return jumpTo.get();
}

} // namespace ast

