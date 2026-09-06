#ifndef EXPRESSIONSTATEMENT_H_
#define EXPRESSIONSTATEMENT_H_

#include <memory>

#include "Expression.h"
#include "Statement.h"

namespace ast {

class ExpressionStatement: public Statement {
public:
    explicit ExpressionStatement(std::unique_ptr<Expression> expression);
    virtual ~ExpressionStatement() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    const std::unique_ptr<Expression> expression;
};

} // namespace ast

#endif
