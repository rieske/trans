#ifndef RETURNSTATEMENT_H_
#define RETURNSTATEMENT_H_

#include <memory>

#include "ast/Expression.h"
#include "ast/Statement.h"

namespace ast {

class ReturnStatement: public Statement {
public:
    explicit ReturnStatement(std::unique_ptr<Expression> returnExpression = nullptr);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    std::unique_ptr<Expression> returnExpression;
};

} // namespace ast

#endif // RETURNSTATEMENT_H_
