#include "CaseLabel.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
CaseLabel::CaseLabel(std::unique_ptr<Expression> caseExpression, std::unique_ptr<AbstractSyntaxTreeNode> statement)
    : caseExpression{std::move(caseExpression)}, statement{std::move(statement)} {}
void CaseLabel::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void CaseLabel::setLabel(symbols::LabelEntry label) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Primary, std::move(label));
}
symbols::LabelEntry* CaseLabel::getLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Primary);
}
void CaseLabel::setCaseValue(long value) { caseValue = value; }
long CaseLabel::getCaseValue() const { return caseValue; }
} // namespace ast
