#include "PrefixExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

PrefixExpression::PrefixExpression(std::string lexeme, std::unique_ptr<Expression> unaryExpression) :
        UnaryOpExpression(std::move(unaryExpression), std::move(lexeme)) {
}

PrefixExpression::~PrefixExpression() {
}

void PrefixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

