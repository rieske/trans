#include "AssignmentExpressionList.h"

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string AssignmentExpressionList::ID { "<a_expressions>" };

AssignmentExpressionList::AssignmentExpressionList() {
}

AssignmentExpressionList::AssignmentExpressionList(std::unique_ptr<Expression> expression) {
    expressions.push_back(std::move(expression));
}

void AssignmentExpressionList::addExpression(std::unique_ptr<Expression> expression) {
    expressions.push_back(std::move(expression));
}

const std::vector<std::unique_ptr<Expression>>& AssignmentExpressionList::getExpressions() const {
    return expressions;
}

void AssignmentExpressionList::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
