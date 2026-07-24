#include "SwitchStatement.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "CaseLabel.h"
#include "DefaultLabel.h"
#include "symbols/AnnotationStore.h"
namespace ast {
SwitchStatement::SwitchStatement(std::unique_ptr<Expression> expression, std::unique_ptr<AbstractSyntaxTreeNode> body)
    : expression{std::move(expression)}, body{std::move(body)} {}
void SwitchStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void SwitchStatement::setExitLabel(symbols::LabelEntry exitLabel) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Exit, std::move(exitLabel));
}
symbols::LabelEntry* SwitchStatement::getExitLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Exit);
}
void SwitchStatement::setCaseTemp(symbols::ValueEntry temp) {
    symbols::AnnotationStore::current().setValue(this, symbols::ValueSlot::CaseTemp, std::move(temp));
}
symbols::ValueEntry* SwitchStatement::getCaseTemp() const {
    return symbols::AnnotationStore::current().value(this, symbols::ValueSlot::CaseTemp);
}
void SwitchStatement::addCase(CaseLabel* caseLabel) { cases.push_back(caseLabel); }
const std::vector<CaseLabel*>& SwitchStatement::getCases() const { return cases; }
void SwitchStatement::setDefaultLabel(DefaultLabel* defaultLabel) { defaultLabelNode = defaultLabel; }
DefaultLabel* SwitchStatement::getDefaultLabel() const { return defaultLabelNode; }
} // namespace ast
