#include "CaseLabel.h"
#include "AbstractSyntaxTreeVisitor.h"
namespace ast {
CaseLabel::CaseLabel(std::unique_ptr<Expression> caseExpression, std::unique_ptr<AbstractSyntaxTreeNode> statement)
    : caseExpression{std::move(caseExpression)}, statement{std::move(statement)} {}
void CaseLabel::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void CaseLabel::setCaseValue(long value) { caseValue = value; }
long CaseLabel::getCaseValue() const { return caseValue; }
} // namespace ast
