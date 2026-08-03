#include "SwitchStatement.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "CaseLabel.h"
#include "DefaultLabel.h"
namespace ast {
SwitchStatement::SwitchStatement(std::unique_ptr<Expression> expression, std::unique_ptr<AbstractSyntaxTreeNode> body)
    : expression{std::move(expression)}, body{std::move(body)} {}
void SwitchStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void SwitchStatement::addCase(CaseLabel* caseLabel) { cases.push_back(caseLabel); }
const std::vector<CaseLabel*>& SwitchStatement::getCases() const { return cases; }
void SwitchStatement::setDefaultLabel(DefaultLabel* defaultLabel) { defaultLabelNode = defaultLabel; }
DefaultLabel* SwitchStatement::getDefaultLabel() const { return defaultLabelNode; }
} // namespace ast
