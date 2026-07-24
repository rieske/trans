#ifndef DOWHILELOOPHEADER_H_
#define DOWHILELOOPHEADER_H_

#include <memory>

#include "ast/LoopHeader.h"

namespace ast {

class DoWhileLoopHeader: public LoopHeader {
public:
    explicit DoWhileLoopHeader(std::unique_ptr<Expression> clause);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    // Body runs before the condition; continue jumps to the test, not the body entry.
    bool bodyBeforeTest() const override { return true; }
    bool continueTargetsEntry() const override { return false; }

    const std::unique_ptr<Expression> clause;
};

} // namespace ast

#endif // DOWHILELOOPHEADER_H_
