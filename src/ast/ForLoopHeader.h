#ifndef FORLOOPHEADER_H_
#define FORLOOPHEADER_H_

#include <memory>

#include "LoopHeader.h"

namespace ast {

// for-init is either an expression, a declaration (C99), or absent — one optional AST node.
class ForLoopHeader: public LoopHeader {
public:
    ForLoopHeader(std::unique_ptr<AbstractSyntaxTreeNode> initialization,
            std::unique_ptr<Expression> clause,
            std::unique_ptr<Expression> increment,
            bool declarationScoped = false);
    virtual ~ForLoopHeader();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    bool opensBlockScope() const override { return declarationScoped; }

    const std::unique_ptr<AbstractSyntaxTreeNode> initialization;
    const std::unique_ptr<Expression> clause;

private:
    const bool declarationScoped;
};

} // namespace ast

#endif // FORLOOPHEADER_H_
