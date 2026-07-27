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

void JumpStatement::setJumpTo(symbols::AnnotationStore& store, symbols::LabelEntry label) {
    store.setLabel(this, symbols::LabelSlot::Target, std::move(label));
}

symbols::LabelEntry* JumpStatement::getJumpTo(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Target);
}

} // namespace ast

