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

bool JumpStatement::isBreak() const {
	return jumpKeyword.type == "break";
}

void JumpStatement::setTarget(semantic_analyzer::LabelEntry target) {
	this->target = std::make_unique<semantic_analyzer::LabelEntry>(target);
}

semantic_analyzer::LabelEntry* JumpStatement::getTarget() const {
	return target.get();
}

} // namespace ast
