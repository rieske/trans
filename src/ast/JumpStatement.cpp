#include "JumpStatement.h"

#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

namespace {

JumpKind kindFromKeyword(const std::string& value) {
	if (value == "break") {
		return JumpKind::Break;
	}
	if (value == "continue") {
		return JumpKind::Continue;
	}
	throw std::runtime_error { "bad loop jump keyword: " + value };
}

}

JumpStatement::JumpStatement(TerminalSymbol jumpKeyword) :
		jumpKeyword { jumpKeyword },
		kind_ { kindFromKeyword(this->jumpKeyword.value) } {
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

