#ifndef _IO_STMT_NODE_H_
#define _IO_STMT_NODE_H_

#include <memory>
#include <string>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"
#include "ast/Expression.h"

namespace ast {

class IOStatement: public AbstractSyntaxTreeNode {
public:
    IOStatement(TerminalSymbol ioKeyword, std::unique_ptr<Expression> expression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

    TerminalSymbol ioKeyword;
    std::unique_ptr<Expression> expression;
};

} // namespace ast

#endif // _IO_STMT_NODE_H_
