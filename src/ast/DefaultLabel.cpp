#include "DefaultLabel.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

DefaultLabel::DefaultLabel(TerminalSymbol defaultKeyword, std::unique_ptr<AbstractSyntaxTreeNode> statement) :
        defaultKeyword { std::move(defaultKeyword) },
        statement { std::move(statement) } {
}

void DefaultLabel::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void DefaultLabel::setLabel(symbols::AnnotationStore& store, symbols::LabelEntry label) {
    store.setLabel(this, symbols::LabelSlot::Primary, std::move(label));
}

symbols::LabelEntry* DefaultLabel::getLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Primary);
}

} // namespace ast
