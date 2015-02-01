#include "ArgumentExpressionList.h"

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string ArgumentExpressionList::ID { "<argument_exp_list>" };

ArgumentExpressionList::ArgumentExpressionList() {
}

ArgumentExpressionList::ArgumentExpressionList(std::unique_ptr<Expression> expression) {
    expressions.push_back(std::move(expression));
}

void ArgumentExpressionList::addExpression(std::unique_ptr<Expression> expression) {
    expressions.push_back(std::move(expression));
}

const std::vector<std::unique_ptr<Expression>>& ArgumentExpressionList::getExpressions() const {
    return expressions;
}

void ArgumentExpressionList::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
