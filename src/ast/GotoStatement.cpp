#include "GotoStatement.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

GotoStatement::GotoStatement(TerminalSymbol gotoKeyword, TerminalSymbol labelName) :
        gotoKeyword { gotoKeyword },
        label { labelName } {
}

void GotoStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void GotoStatement::setTarget(symbols::AnnotationStore& store, symbols::LabelEntry target) {
    store.setLabel(this, symbols::LabelSlot::Target, std::move(target));
}

symbols::LabelEntry* GotoStatement::getTarget(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Target);
}

const std::string& GotoStatement::getLabelName() const {
    return label.value;
}

} // namespace ast
