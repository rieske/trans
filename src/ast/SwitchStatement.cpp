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

void SwitchStatement::setExitLabel(semantic_analyzer::LabelEntry exitLabel) {
    this->exitLabel = std::make_unique<semantic_analyzer::LabelEntry>(exitLabel);
}

semantic_analyzer::LabelEntry* SwitchStatement::getExitLabel() const {
    return exitLabel.get();
}

void SwitchStatement::setCaseTemp(semantic_analyzer::ValueEntry temp) {
    this->caseTemp = std::make_unique<semantic_analyzer::ValueEntry>(temp);
}

semantic_analyzer::ValueEntry* SwitchStatement::getCaseTemp() const {
    return caseTemp.get();
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
