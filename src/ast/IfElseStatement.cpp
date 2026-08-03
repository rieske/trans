#include "IfElseStatement.h"
#include "AbstractSyntaxTreeVisitor.h"
namespace ast {
IfElseStatement::IfElseStatement(std::unique_ptr<Expression> testExpression,
        std::unique_ptr<AbstractSyntaxTreeNode> truthyBody, std::unique_ptr<AbstractSyntaxTreeNode> falsyBody)
    : testExpression{std::move(testExpression)}, truthyBody{std::move(truthyBody)}, falsyBody{std::move(falsyBody)} {}
IfElseStatement::~IfElseStatement() = default;
void IfElseStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
} // namespace ast
