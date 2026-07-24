#include "CaseLabel.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

CaseLabel::CaseLabel(std::unique_ptr<Expression> caseExpression, std::unique_ptr<AbstractSyntaxTreeNode> statement) :
        caseExpression { std::move(caseExpression) },
        statement { std::move(statement) } {
}

void CaseLabel::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void CaseLabel::setLabel(semantic_analyzer::LabelEntry label) {
    this->label = std::make_unique<semantic_analyzer::LabelEntry>(label);
}

semantic_analyzer::LabelEntry* CaseLabel::getLabel() const {
    return label.get();
}

void CaseLabel::setCaseValue(long value) {
    caseValue = value;
}

long CaseLabel::getCaseValue() const {
    return caseValue;
}

} // namespace ast
