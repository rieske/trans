#include "DefaultLabel.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
DefaultLabel::DefaultLabel(TerminalSymbol defaultKeyword, std::unique_ptr<AbstractSyntaxTreeNode> statement)
    : defaultKeyword{std::move(defaultKeyword)}, statement{std::move(statement)} {}
void DefaultLabel::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void DefaultLabel::setLabel(symbols::LabelEntry label) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Primary, std::move(label));
}
symbols::LabelEntry* DefaultLabel::getLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Primary);
}
} // namespace ast
