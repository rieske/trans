#include "AssignmentExpressionList.h"

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string AssignmentExpressionList::ID { "<a_expressions>" };

AssignmentExpressionList::AssignmentExpressionList() {
}

AssignmentExpressionList::AssignmentExpressionList(std::unique_ptr<Expression> expression) {
    code = expression->getCode();

    expressions.push_back(std::move(expression));
}

void AssignmentExpressionList::addExpression(std::unique_ptr<Expression> expression) {
    vector<Quadruple *> expressionCode = expression->getCode();
    code.insert(code.end(), expressionCode.begin(), expressionCode.end());

    expressions.push_back(std::move(expression));
}

const vector<std::unique_ptr<Expression>>& AssignmentExpressionList::getExpressions() const {
    return expressions;
}

void AssignmentExpressionList::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
