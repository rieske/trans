#include "LabeledStatement.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

LabeledStatement::LabeledStatement(TerminalSymbol labelName, std::unique_ptr<AbstractSyntaxTreeNode> statement) :
        name { labelName },
        statement { std::move(statement) } {
}

void LabeledStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void LabeledStatement::setLabel(symbols::AnnotationStore& store, symbols::LabelEntry label) {
    store.setLabel(this, symbols::LabelSlot::Primary, std::move(label));
}

symbols::LabelEntry* LabeledStatement::getLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Primary);
}

const std::string& LabeledStatement::getLabelName() const {
    return name.value;
}

} // namespace ast
