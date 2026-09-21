#ifndef FORLOOPHEADER_H_
#define FORLOOPHEADER_H_

#include <memory>

#include "ForInit.h"
#include "LoopHeader.h"

namespace ast {

class ForLoopHeader: public LoopHeader {
public:
    ForLoopHeader(ForInit initialization,
            std::unique_ptr<Expression> clause,
            std::unique_ptr<Expression> increment);
    LoopKind loopKind() const override { return LoopKind::For; }
    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    ForInit initialization;
    std::unique_ptr<Expression> clause;
};

} // namespace ast

#endif // FORLOOPHEADER_H_
