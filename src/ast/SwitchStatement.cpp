#include "SwitchStatement.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "CaseLabel.h"
#include "DefaultLabel.h"

namespace ast {

SwitchStatement::SwitchStatement(std::unique_ptr<Expression> expression, std::unique_ptr<AbstractSyntaxTreeNode> body) :
        expression { std::move(expression) },
        body { std::move(body) } {
}

void SwitchStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void SwitchStatement::setExitLabel(symbols::AnnotationStore& store, symbols::LabelEntry exitLabel) {
    store.setLabel(this, symbols::LabelSlot::Exit, std::move(exitLabel));
}

symbols::LabelEntry* SwitchStatement::getExitLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Exit);
}

void SwitchStatement::setCaseTemp(symbols::AnnotationStore& store, symbols::ValueEntry temp) {
    store.setCaseTemp(this, std::move(temp));
}

symbols::ValueEntry* SwitchStatement::getCaseTemp(symbols::AnnotationStore& store) const {
    return store.caseTemp(this);
}

void SwitchStatement::addCase(CaseLabel* caseLabel) {
    cases.push_back(caseLabel);
}

const std::vector<CaseLabel*>& SwitchStatement::getCases() const {
    return cases;
}

void SwitchStatement::setDefaultLabel(DefaultLabel* defaultLabel) {
    defaultLabelNode = defaultLabel;
}

DefaultLabel* SwitchStatement::getDefaultLabel() const {
    return defaultLabelNode;
}

} // namespace ast
