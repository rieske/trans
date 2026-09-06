#ifndef RETURNSTATEMENT_H_
#define RETURNSTATEMENT_H_

#include <memory>

#include "ast/Expression.h"
#include "ast/Statement.h"

namespace ast {

class ReturnStatement: public Statement {
public:
    ReturnStatement(std::unique_ptr<Expression> returnExpression);
    virtual ~ReturnStatement() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    std::unique_ptr<Expression> returnExpression;
};

} // namespace ast

#endif // RETURNSTATEMENT_H_
