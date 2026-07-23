#ifndef DOWHILELOOPHEADER_H_
#define DOWHILELOOPHEADER_H_

#include <memory>

#include "ast/LoopHeader.h"

namespace ast {

class DoWhileLoopHeader: public LoopHeader {
public:
    explicit DoWhileLoopHeader(std::unique_ptr<Expression> clause);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    bool bodyBeforeTest() const override { return true; }
    Expression* testExpression() override { return clause.get(); }
    const Expression* testExpression() const override { return clause.get(); }

    const std::unique_ptr<Expression> clause;
};

} // namespace ast

#endif // DOWHILELOOPHEADER_H_
