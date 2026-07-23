#ifndef FORLOOPHEADER_H_
#define FORLOOPHEADER_H_

#include <memory>

#include "LoopHeader.h"
#include "Declaration.h"

namespace ast {

class ForLoopHeader: public LoopHeader {
public:
    ForLoopHeader(std::unique_ptr<Expression> initialization,
            std::unique_ptr<Expression> clause,
            std::unique_ptr<Expression> increment);
    ForLoopHeader(std::unique_ptr<Declaration> declarationInit,
            std::unique_ptr<Expression> clause,
            std::unique_ptr<Expression> increment);
    virtual ~ForLoopHeader();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    // Exactly one of initialization / declarationInit is used (or neither).
    const std::unique_ptr<Expression> initialization;
    const std::unique_ptr<Declaration> declarationInit;
    const std::unique_ptr<Expression> clause;
};

} // namespace ast

#endif // FORLOOPHEADER_H_
