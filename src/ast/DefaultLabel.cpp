#include "DefaultLabel.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

DefaultLabel::DefaultLabel(std::unique_ptr<AbstractSyntaxTreeNode> statement) :
        statement { std::move(statement) } {
}

void DefaultLabel::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void DefaultLabel::setLabel(semantic_analyzer::LabelEntry label) {
    this->label = std::make_unique<semantic_analyzer::LabelEntry>(label);
}

semantic_analyzer::LabelEntry* DefaultLabel::getLabel() const {
    return label.get();
}

} // namespace ast
