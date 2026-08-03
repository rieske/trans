#include "IfStatement.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

IfStatement::IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body)
    : testExpression{std::move(testExpression)}, body{std::move(body)} {}

IfStatement::~IfStatement() = default;

void IfStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }


} // namespace ast
