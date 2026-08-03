#ifndef WHILELOOPHEADER_H_
#define WHILELOOPHEADER_H_

#include <memory>

#include "ast/LoopHeader.h"

namespace ast {

class WhileLoopHeader: public LoopHeader {
public:
    WhileLoopHeader(std::unique_ptr<Expression> clause);
    virtual ~WhileLoopHeader();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    Expression* testExpression() override { return clause.get(); }
    const Expression* testExpression() const override { return clause.get(); }

    const std::unique_ptr<Expression> clause;
};

} // namespace ast

#endif // WHILELOOPHEADER_H_
