#include "GotoStatement.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
GotoStatement::GotoStatement(TerminalSymbol gotoKeyword, TerminalSymbol labelName)
    : gotoKeyword{std::move(gotoKeyword)}, label{std::move(labelName)} {}
void GotoStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void GotoStatement::setTarget(symbols::LabelEntry target) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Target, std::move(target));
}
symbols::LabelEntry* GotoStatement::getTarget() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Target);
}
const std::string& GotoStatement::getLabelName() const { return label.value; }
} // namespace ast
