#include "ExpressionStatement.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ExpressionStatement::ExpressionStatement(std::unique_ptr<Expression> expression) :
        expression { std::move(expression) } {
}

void ExpressionStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast
