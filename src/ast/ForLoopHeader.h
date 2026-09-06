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
    virtual ~ForLoopHeader();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    bool opensBlockScope() const override { return initialization.asDeclaration() != nullptr; }

    const ForInit initialization;
    const std::unique_ptr<Expression> clause;
};

} // namespace ast

#endif // FORLOOPHEADER_H_
