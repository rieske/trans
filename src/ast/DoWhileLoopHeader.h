#ifndef DOWHILELOOPHEADER_H_
#define DOWHILELOOPHEADER_H_

#include <memory>

#include "ast/LoopHeader.h"

namespace ast {

class DoWhileLoopHeader: public LoopHeader {
public:
    DoWhileLoopHeader(std::unique_ptr<Expression> clause);
    virtual ~DoWhileLoopHeader() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    const std::unique_ptr<Expression> clause;
};

} // namespace ast

#endif // DOWHILELOOPHEADER_H_
