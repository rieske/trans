#ifndef FORLOOPHEADER_H_
#define FORLOOPHEADER_H_

#include <memory>

#include "AbstractSyntaxTreeNode.h"
#include "LoopHeader.h"

namespace ast {

// for-init is one optional AST node: Expression, Declaration (C99), or null.
class ForLoopHeader: public LoopHeader {
public:
    ForLoopHeader(std::unique_ptr<AbstractSyntaxTreeNode> initialization,
            std::unique_ptr<Expression> clause,
            std::unique_ptr<Expression> increment);
    virtual ~ForLoopHeader();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    bool isDeclarationInit() const;

    Expression* testExpression() override { return clause.get(); }
    const Expression* testExpression() const override { return clause.get(); }

    const std::unique_ptr<AbstractSyntaxTreeNode> initialization;
    const std::unique_ptr<Expression> clause;
};

} // namespace ast

#endif // FORLOOPHEADER_H_
