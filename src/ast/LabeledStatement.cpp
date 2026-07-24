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

void LabeledStatement::setLabel(semantic_analyzer::LabelEntry label) {
    this->label = std::make_unique<semantic_analyzer::LabelEntry>(label);
}

semantic_analyzer::LabelEntry* LabeledStatement::getLabel() const {
    return label.get();
}

const std::string& LabeledStatement::getLabelName() const {
    return name.value;
}

} // namespace ast
